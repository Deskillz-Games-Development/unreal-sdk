// Copyright Deskillz Games. All Rights Reserved.

#include "Security/DeskillzScoreEncryption.h"
#include "Deskillz.h"
#include "Misc/Base64.h"
#include "Misc/SecureHash.h"
#include "Misc/Guid.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "GenericPlatform/GenericPlatformMisc.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformMisc.h"
#elif PLATFORM_MAC
#include "Mac/MacPlatformMisc.h"
#elif PLATFORM_IOS
#include "IOS/IOSPlatformMisc.h"
#elif PLATFORM_ANDROID
#include "Android/AndroidPlatformMisc.h"
#endif

// OpenSSL for AES-GCM (when available)
#if WITH_SSL
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#endif

// ============================================================================
// FDeskillzEncryptedScore Implementation
// ============================================================================

FString FDeskillzEncryptedScore::ToJson() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	
	JsonObject->SetStringField(TEXT("encryptedData"), EncryptedData);
	JsonObject->SetStringField(TEXT("iv"), IV);
	JsonObject->SetStringField(TEXT("authTag"), AuthTag);
	JsonObject->SetStringField(TEXT("signature"), Signature);
	JsonObject->SetNumberField(TEXT("timestamp"), Timestamp);
	JsonObject->SetStringField(TEXT("nonce"), Nonce);
	JsonObject->SetStringField(TEXT("matchId"), MatchId);
	JsonObject->SetStringField(TEXT("deviceFingerprint"), DeviceFingerprint);
	JsonObject->SetNumberField(TEXT("version"), Version);
	
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	
	return OutputString;
}

FDeskillzEncryptedScore FDeskillzEncryptedScore::FromJson(const FString& JsonString)
{
	FDeskillzEncryptedScore Result;
	
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		JsonObject->TryGetStringField(TEXT("encryptedData"), Result.EncryptedData);
		JsonObject->TryGetStringField(TEXT("iv"), Result.IV);
		JsonObject->TryGetStringField(TEXT("authTag"), Result.AuthTag);
		JsonObject->TryGetStringField(TEXT("signature"), Result.Signature);
		JsonObject->TryGetNumberField(TEXT("timestamp"), Result.Timestamp);
		JsonObject->TryGetStringField(TEXT("nonce"), Result.Nonce);
		JsonObject->TryGetStringField(TEXT("matchId"), Result.MatchId);
		JsonObject->TryGetStringField(TEXT("deviceFingerprint"), Result.DeviceFingerprint);
		
		int32 VersionInt;
		if (JsonObject->TryGetNumberField(TEXT("version"), VersionInt))
		{
			Result.Version = VersionInt;
		}
	}
	
	return Result;
}

// ============================================================================
// UDeskillzScoreEncryption Implementation
// ============================================================================

UDeskillzScoreEncryption::UDeskillzScoreEncryption()
{
	SessionStartTime = GetTimestampMs();
}

void UDeskillzScoreEncryption::Initialize(const FString& InEncryptionKey, const FString& InSigningKey, const FString& MatchId)
{
	// Decode keys from Base64
	EncryptionKey = FromBase64(InEncryptionKey);
	SigningKey = FromBase64(InSigningKey);
	CurrentMatchId = MatchId;
	
	// Validate key lengths
	if (EncryptionKey.Num() != 32) // AES-256 requires 32 bytes
	{
		UE_LOG(LogDeskillz, Error, TEXT("Invalid encryption key length: %d (expected 32)"), EncryptionKey.Num());
		bIsInitialized = false;
		return;
	}
	
	// Reset state
	ScoreTrail.Empty();
	RunningScoreHash = TEXT("");
	InputEventCount = 0;
	SessionStartTime = GetTimestampMs();
	FrameCount = 0;
	FPSSamples.Empty();
	UsedNonces.Empty();
	
	bIsInitialized = true;
	
	UE_LOG(LogDeskillz, Log, TEXT("Score encryption initialized for match: %s"), *MatchId);
}

void UDeskillzScoreEncryption::Reset()
{
	EncryptionKey.Empty();
	SigningKey.Empty();
	CurrentMatchId.Empty();
	ScoreTrail.Empty();
	RunningScoreHash.Empty();
	InputEventCount = 0;
	UsedNonces.Empty();
	bIsInitialized = false;
	
	UE_LOG(LogDeskillz, Log, TEXT("Score encryption reset"));
}

FDeskillzEncryptedScore UDeskillzScoreEncryption::EncryptScore(int64 Score, const FDeskillzScoreIntegrity& Integrity)
{
	FDeskillzEncryptedScore Result;
	
	if (!bIsInitialized)
	{
		UE_LOG(LogDeskillz, Error, TEXT("Cannot encrypt score: encryption not initialized"));
		return Result;
	}
	
	// Build payload JSON
	TSharedPtr<FJsonObject> PayloadJson = MakeShared<FJsonObject>();
	PayloadJson->SetNumberField(TEXT("score"), Score);
	PayloadJson->SetNumberField(TEXT("randomSeed"), Integrity.RandomSeed);
	PayloadJson->SetNumberField(TEXT("sessionDurationMs"), Integrity.SessionDurationMs);
	PayloadJson->SetNumberField(TEXT("scoreUpdateCount"), Integrity.ScoreUpdateCount);
	PayloadJson->SetStringField(TEXT("gameStateHash"), Integrity.GameStateHash);
	PayloadJson->SetNumberField(TEXT("inputEventCount"), Integrity.InputEventCount);
	PayloadJson->SetNumberField(TEXT("frameCount"), Integrity.FrameCount);
	PayloadJson->SetNumberField(TEXT("averageFPS"), Integrity.AverageFPS);
	PayloadJson->SetStringField(TEXT("scoreTrailHash"), Integrity.ScoreTrailHash);
	
	FString PayloadString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadString);
	FJsonSerializer::Serialize(PayloadJson.ToSharedRef(), Writer);
	
	// Convert to bytes
	TArray<uint8> PlaintextBytes;
	FTCHARToUTF8 Converter(*PayloadString);
	PlaintextBytes.Append((uint8*)Converter.Get(), Converter.Length());
	
	// Generate IV (12 bytes for GCM)
	TArray<uint8> IV = GenerateIV();
	
	// Generate nonce
	FString Nonce = GenerateNonce();
	
	// Prevent nonce reuse
	if (UsedNonces.Contains(Nonce))
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Nonce collision detected, regenerating"));
		Nonce = GenerateNonce();
	}
	UsedNonces.Add(Nonce);
	
	// Additional authenticated data (AAD)
	FString AADString = FString::Printf(TEXT("%s|%s|%lld"), *CurrentMatchId, *Nonce, GetTimestampMs());
	TArray<uint8> AADBytes;
	FTCHARToUTF8 AADConverter(*AADString);
	AADBytes.Append((uint8*)AADConverter.Get(), AADConverter.Length());
	
	// Encrypt with AES-256-GCM
	TArray<uint8> Ciphertext;
	TArray<uint8> AuthTag;
	
	if (!AES256GCMEncrypt(PlaintextBytes, EncryptionKey, IV, AADBytes, Ciphertext, AuthTag))
	{
		UE_LOG(LogDeskillz, Error, TEXT("AES-256-GCM encryption failed"));
		return Result;
	}
	
	// Build result
	Result.EncryptedData = ToBase64(Ciphertext);
	Result.IV = ToBase64(IV);
	Result.AuthTag = ToBase64(AuthTag);
	Result.Timestamp = GetTimestampMs();
	Result.Nonce = Nonce;
	Result.MatchId = CurrentMatchId;
	Result.DeviceFingerprint = GetDeviceFingerprint();
	Result.Version = 1;
	
	// Sign the payload
	FString SigningPayload = BuildSigningPayload(Result.EncryptedData, Result.IV, Result.Timestamp, Result.Nonce);
	
	TArray<uint8> SigningPayloadBytes;
	FTCHARToUTF8 SigningConverter(*SigningPayload);
	SigningPayloadBytes.Append((uint8*)SigningConverter.Get(), SigningConverter.Length());
	
	TArray<uint8> SignatureBytes = HMACSHA256(SigningPayloadBytes, SigningKey);
	Result.Signature = ToBase64(SignatureBytes);
	
	UE_LOG(LogDeskillz, Log, TEXT("Score %lld encrypted successfully"), Score);
	
	return Result;
}

FDeskillzEncryptedScore UDeskillzScoreEncryption::EncryptScoreSimple(int64 Score)
{
	FDeskillzScoreIntegrity Integrity = CollectIntegrityData(Score);
	return EncryptScore(Score, Integrity);
}

bool UDeskillzScoreEncryption::VerifySignature(const FDeskillzEncryptedScore& EncryptedScore) const
{
	if (!bIsInitialized || SigningKey.Num() == 0)
	{
		return false;
	}
	
	// Rebuild signing payload
	FString SigningPayload = BuildSigningPayload(
		EncryptedScore.EncryptedData, 
		EncryptedScore.IV, 
		EncryptedScore.Timestamp, 
		EncryptedScore.Nonce
	);
	
	TArray<uint8> SigningPayloadBytes;
	FTCHARToUTF8 Converter(*SigningPayload);
	SigningPayloadBytes.Append((uint8*)Converter.Get(), Converter.Length());
	
	// Compute expected signature
	TArray<uint8> ExpectedSignature = HMACSHA256(SigningPayloadBytes, SigningKey);
	FString ExpectedSignatureBase64 = ToBase64(ExpectedSignature);
	
	// Compare (constant-time comparison for security)
	if (ExpectedSignatureBase64.Len() != EncryptedScore.Signature.Len())
	{
		return false;
	}
	
	int32 Diff = 0;
	for (int32 i = 0; i < ExpectedSignatureBase64.Len(); i++)
	{
		Diff |= ExpectedSignatureBase64[i] ^ EncryptedScore.Signature[i];
	}
	
	return Diff == 0;
}

void UDeskillzScoreEncryption::RecordScoreUpdate(int64 NewScore)
{
	int64 Timestamp = GetTimestampMs();
	ScoreTrail.Add(TPair<int64, int64>(Timestamp, NewScore));
	UpdateScoreHash(NewScore, Timestamp);
	
	// Track frame count
	FrameCount++;
}

void UDeskillzScoreEncryption::RecordInputEvent(const FString& InputType, const FString& InputData)
{
	InputEventCount++;
	
	// Include in score hash for verification
	FString InputHash = FString::Printf(TEXT("%s|%s|%lld"), *InputType, *InputData, GetTimestampMs());
	RunningScoreHash = HashSHA256(RunningScoreHash + InputHash);
}

FString UDeskillzScoreEncryption::GetScoreTrailHash() const
{
	return RunningScoreHash;
}

FString UDeskillzScoreEncryption::GetDeviceFingerprint() const
{
	// Build fingerprint from device characteristics
	FString Fingerprint;
	
	// Device ID
	Fingerprint += GetDeviceId();
	
	// Platform info
	Fingerprint += TEXT("|");
	Fingerprint += FPlatformMisc::GetOSVersion();
	
	// CPU info
	Fingerprint += TEXT("|");
	Fingerprint += FPlatformMisc::GetCPUBrand();
	
	// GPU info
	Fingerprint += TEXT("|");
	Fingerprint += GRHIAdapterName;
	
	return HashSHA256(Fingerprint);
}

FString UDeskillzScoreEncryption::GetDeviceId()
{
	// Get persistent device identifier
	FString DeviceId;
	
#if PLATFORM_WINDOWS
	DeviceId = FWindowsPlatformMisc::GetMachineId().ToString();
#elif PLATFORM_MAC
	DeviceId = FMacPlatformMisc::GetMachineId().ToString();
#elif PLATFORM_IOS
	DeviceId = FIOSPlatformMisc::GetDeviceId();
#elif PLATFORM_ANDROID
	DeviceId = FAndroidMisc::GetDeviceId();
#else
	// Fallback: generate from login ID
	DeviceId = FPlatformMisc::GetLoginId();
#endif
	
	if (DeviceId.IsEmpty())
	{
		// Last resort: use MAC address hash
		DeviceId = FPlatformMisc::GetMacAddressString();
	}
	
	// Hash for privacy
	return HashSHA256(DeviceId);
}

FString UDeskillzScoreEncryption::GenerateNonce()
{
	// Generate 16 random bytes
	TArray<uint8> NonceBytes;
	NonceBytes.SetNumUninitialized(16);
	
#if WITH_SSL
	RAND_bytes(NonceBytes.GetData(), 16);
#else
	// Fallback: use FGuid
	FGuid Guid = FGuid::NewGuid();
	FMemory::Memcpy(NonceBytes.GetData(), &Guid, 16);
#endif
	
	return ToBase64(NonceBytes);
}

int64 UDeskillzScoreEncryption::GetTimestampMs()
{
	return FDateTime::UtcNow().ToUnixTimestamp() * 1000 + 
		   (FDateTime::UtcNow().GetMillisecond());
}

FString UDeskillzScoreEncryption::HashSHA256(const FString& Data)
{
	FSHAHash Hash;
	FSHA1 Sha1;
	
	TArray<uint8> DataBytes;
	FTCHARToUTF8 Converter(*Data);
	DataBytes.Append((uint8*)Converter.Get(), Converter.Length());
	
	// Use SHA-256 via platform crypto
	uint8 HashBytes[32];
	
#if WITH_SSL
	SHA256(DataBytes.GetData(), DataBytes.Num(), HashBytes);
#else
	// Fallback: use Unreal's FSHA1 (SHA-1, less secure but available)
	Sha1.Update(DataBytes.GetData(), DataBytes.Num());
	Sha1.Final();
	Sha1.GetHash(Hash.Hash);
	FMemory::Memcpy(HashBytes, Hash.Hash, 20);
	FMemory::Memset(HashBytes + 20, 0, 12); // Pad to 32 bytes
#endif
	
	// Convert to hex string
	FString HexString;
	for (int32 i = 0; i < 32; i++)
	{
		HexString += FString::Printf(TEXT("%02x"), HashBytes[i]);
	}
	
	return HexString;
}

FString UDeskillzScoreEncryption::ToBase64(const TArray<uint8>& Bytes)
{
	return FBase64::Encode(Bytes);
}

TArray<uint8> UDeskillzScoreEncryption::FromBase64(const FString& Base64String)
{
	TArray<uint8> Result;
	FBase64::Decode(Base64String, Result);
	return Result;
}

// ============================================================================
// Internal Methods
// ============================================================================

bool UDeskillzScoreEncryption::AES256GCMEncrypt(
	const TArray<uint8>& Plaintext,
	const TArray<uint8>& Key,
	const TArray<uint8>& IV,
	const TArray<uint8>& AAD,
	TArray<uint8>& Ciphertext,
	TArray<uint8>& AuthTag)
{
#if WITH_SSL
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (!ctx)
	{
		return false;
	}
	
	bool bSuccess = false;
	int32 len;
	int32 ciphertext_len;
	
	// Initialize encryption
	if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1)
	{
		goto cleanup;
	}
	
	// Set IV length (12 bytes for GCM)
	if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV.Num(), NULL) != 1)
	{
		goto cleanup;
	}
	
	// Set key and IV
	if (EVP_EncryptInit_ex(ctx, NULL, NULL, Key.GetData(), IV.GetData()) != 1)
	{
		goto cleanup;
	}
	
	// Set AAD
	if (AAD.Num() > 0)
	{
		if (EVP_EncryptUpdate(ctx, NULL, &len, AAD.GetData(), AAD.Num()) != 1)
		{
			goto cleanup;
		}
	}
	
	// Encrypt
	Ciphertext.SetNumUninitialized(Plaintext.Num() + 16); // Extra space for padding
	if (EVP_EncryptUpdate(ctx, Ciphertext.GetData(), &len, Plaintext.GetData(), Plaintext.Num()) != 1)
	{
		goto cleanup;
	}
	ciphertext_len = len;
	
	// Finalize
	if (EVP_EncryptFinal_ex(ctx, Ciphertext.GetData() + len, &len) != 1)
	{
		goto cleanup;
	}
	ciphertext_len += len;
	Ciphertext.SetNum(ciphertext_len);
	
	// Get auth tag (16 bytes)
	AuthTag.SetNumUninitialized(16);
	if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, AuthTag.GetData()) != 1)
	{
		goto cleanup;
	}
	
	bSuccess = true;
	
cleanup:
	EVP_CIPHER_CTX_free(ctx);
	return bSuccess;
	
#else
	// Software fallback (XOR-based, less secure)
	// This is a simplified implementation for platforms without OpenSSL
	
	UE_LOG(LogDeskillz, Warning, TEXT("Using software encryption fallback (OpenSSL not available)"));
	
	Ciphertext.SetNum(Plaintext.Num());
	AuthTag.SetNumUninitialized(16);
	
	// Simple XOR encryption (NOT cryptographically secure, just a placeholder)
	for (int32 i = 0; i < Plaintext.Num(); i++)
	{
		Ciphertext[i] = Plaintext[i] ^ Key[i % Key.Num()] ^ IV[i % IV.Num()];
	}
	
	// Generate a simple auth tag
	uint32 Hash = 0;
	for (int32 i = 0; i < Ciphertext.Num(); i++)
	{
		Hash = Hash * 31 + Ciphertext[i];
	}
	FMemory::Memcpy(AuthTag.GetData(), &Hash, sizeof(Hash));
	
	return true;
#endif
}

TArray<uint8> UDeskillzScoreEncryption::HMACSHA256(const TArray<uint8>& Data, const TArray<uint8>& Key)
{
	TArray<uint8> Result;
	Result.SetNumUninitialized(32);
	
#if WITH_SSL
	unsigned int len = 32;
	HMAC(EVP_sha256(), Key.GetData(), Key.Num(), Data.GetData(), Data.Num(), Result.GetData(), &len);
#else
	// Software fallback
	// Simple keyed hash (NOT cryptographically secure)
	uint64 Hash[4] = { 0 };
	
	for (int32 i = 0; i < Data.Num(); i++)
	{
		Hash[i % 4] = Hash[i % 4] * 31 + Data[i] + Key[i % Key.Num()];
	}
	
	FMemory::Memcpy(Result.GetData(), Hash, 32);
#endif
	
	return Result;
}

TArray<uint8> UDeskillzScoreEncryption::GenerateIV()
{
	TArray<uint8> IV;
	IV.SetNumUninitialized(12); // GCM standard IV size
	
#if WITH_SSL
	RAND_bytes(IV.GetData(), 12);
#else
	// Fallback: use time-based random
	FGuid Guid = FGuid::NewGuid();
	FMemory::Memcpy(IV.GetData(), &Guid, 12);
#endif
	
	return IV;
}

FString UDeskillzScoreEncryption::BuildSigningPayload(const FString& EncryptedData, const FString& IVStr, int64 Timestamp, const FString& Nonce)
{
	return FString::Printf(TEXT("%s|%s|%lld|%s|%s"), 
		*EncryptedData, 
		*IVStr, 
		Timestamp, 
		*Nonce,
		*CurrentMatchId);
}

void UDeskillzScoreEncryption::UpdateScoreHash(int64 Score, int64 Timestamp)
{
	FString Update = FString::Printf(TEXT("%lld|%lld"), Score, Timestamp);
	
	if (RunningScoreHash.IsEmpty())
	{
		RunningScoreHash = HashSHA256(Update);
	}
	else
	{
		RunningScoreHash = HashSHA256(RunningScoreHash + TEXT("|") + Update);
	}
}

FDeskillzScoreIntegrity UDeskillzScoreEncryption::CollectIntegrityData(int64 FinalScore)
{
	FDeskillzScoreIntegrity Integrity;
	
	Integrity.Score = FinalScore;
	Integrity.SessionDurationMs = GetTimestampMs() - SessionStartTime;
	Integrity.ScoreUpdateCount = ScoreTrail.Num();
	Integrity.InputEventCount = InputEventCount;
	Integrity.FrameCount = FrameCount;
	Integrity.ScoreTrailHash = GetScoreTrailHash();
	
	// Calculate average FPS
	if (Integrity.SessionDurationMs > 0 && FrameCount > 0)
	{
		Integrity.AverageFPS = (float)FrameCount / ((float)Integrity.SessionDurationMs / 1000.0f);
	}
	
	// Generate game state hash
	FString GameState = FString::Printf(TEXT("%lld|%d|%d|%lld"), 
		FinalScore, 
		Integrity.ScoreUpdateCount, 
		Integrity.InputEventCount,
		Integrity.FrameCount);
	Integrity.GameStateHash = HashSHA256(GameState);
	
	return Integrity;
}
