// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "DeskillzTests.h"

/**
 * Extended Integration Test Suite for Deskillz SDK
 * Tests complete user flows and module interactions
 */

// ============================================================================
// FULL FLOW INTEGRATION TESTS
// ============================================================================

/**
 * Test complete player onboarding flow
 * Registration -> Wallet Setup -> First Tournament
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzOnboardingFlowTest, 
	"Deskillz.Integration.OnboardingFlow", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

/**
 * Test complete match lifecycle
 * Matchmaking -> Match Start -> Gameplay -> Score Submit -> Results
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzMatchLifecycleTest,
	"Deskillz.Integration.MatchLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

/**
 * Test wallet transaction flow
 * Deposit -> Entry Fee -> Prize Distribution -> Withdrawal
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzWalletFlowTest,
	"Deskillz.Integration.WalletFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

/**
 * Test real-time multiplayer flow
 * Connect -> Join Room -> Sync State -> Disconnect
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzMultiplayerFlowTest,
	"Deskillz.Integration.MultiplayerFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

/**
 * Test analytics event flow
 * Track Events -> Batch -> Flush -> Verify
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzAnalyticsFlowTest,
	"Deskillz.Integration.AnalyticsFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// ============================================================================
// STRESS TESTS
// ============================================================================

/**
 * Test SDK under high load
 * Multiple concurrent operations
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzStressTest,
	"Deskillz.Performance.Stress",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::StressFilter)

/**
 * Test memory stability
 * Long-running operations without leaks
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzMemoryTest,
	"Deskillz.Performance.Memory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::StressFilter)

/**
 * Test network resilience
 * Handle disconnections, retries, timeouts
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzNetworkResilienceTest,
	"Deskillz.Performance.NetworkResilience",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::StressFilter)

// ============================================================================
// SECURITY TESTS
// ============================================================================

/**
 * Test encryption integrity across operations
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzEncryptionIntegrityTest,
	"Deskillz.Security.EncryptionIntegrity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

/**
 * Test anti-cheat across gameplay scenarios
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzAntiCheatIntegrityTest,
	"Deskillz.Security.AntiCheatIntegrity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

/**
 * Test token and session security
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzSessionSecurityTest,
	"Deskillz.Security.SessionSecurity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// ============================================================================
// PLATFORM-SPECIFIC TESTS
// ============================================================================

/**
 * Test iOS-specific functionality
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzIOSPlatformTest,
	"Deskillz.Platform.iOS",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

/**
 * Test Android-specific functionality
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzAndroidPlatformTest,
	"Deskillz.Platform.Android",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

/**
 * Test Desktop functionality
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzDesktopPlatformTest,
	"Deskillz.Platform.Desktop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// ============================================================================
// UI TESTS
// ============================================================================

/**
 * Test UI widget creation and destruction
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzUIWidgetTest,
	"Deskillz.UI.Widgets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

/**
 * Test UI navigation flow
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDeskillzUINavigationTest,
	"Deskillz.UI.Navigation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// ============================================================================
// TEST STATE MACHINE
// ============================================================================

/**
 * State machine for complex multi-step tests
 */
class DESKILLZ_API FDeskillzTestStateMachine
{
public:
	enum class ETestState : uint8
	{
		Idle,
		Initializing,
		Running,
		WaitingForCallback,
		Verifying,
		Completed,
		Failed,
		TimedOut
	};

	FDeskillzTestStateMachine();

	/** Start the test state machine */
	void Start();

	/** Update state machine (call each tick) */
	void Tick(float DeltaTime);

	/** Get current state */
	ETestState GetState() const { return CurrentState; }

	/** Check if completed (success or failure) */
	bool IsCompleted() const;

	/** Check if successful */
	bool IsSuccessful() const { return CurrentState == ETestState::Completed; }

	/** Get error message if failed */
	FString GetErrorMessage() const { return ErrorMessage; }

	/** Set timeout duration */
	void SetTimeout(float Seconds) { TimeoutDuration = Seconds; }

	/** Transition to next state */
	void TransitionTo(ETestState NewState);

	/** Fail with error */
	void Fail(const FString& Error);

	/** Mark step complete and advance */
	void CompleteStep();

protected:
	/** Override to define test steps */
	virtual void OnEnterState(ETestState State) {}
	virtual void OnExitState(ETestState State) {}
	virtual void OnTick(float DeltaTime) {}

private:
	ETestState CurrentState;
	FString ErrorMessage;
	float TimeoutDuration;
	float ElapsedTime;
	int32 CurrentStep;
	int32 TotalSteps;
};

// ============================================================================
// ASYNC TEST HELPER
// ============================================================================

/**
 * Helper for async test operations
 */
class DESKILLZ_API FDeskillzAsyncTestHelper
{
public:
	FDeskillzAsyncTestHelper();

	/** Start async operation with timeout */
	void StartAsync(float TimeoutSeconds = 10.0f);

	/** Signal completion */
	void SignalComplete(bool bSuccess = true);

	/** Signal failure */
	void SignalFailure(const FString& Reason);

	/** Check if still waiting */
	bool IsWaiting() const { return bIsWaiting; }

	/** Check if completed successfully */
	bool WasSuccessful() const { return bWasSuccessful; }

	/** Get failure reason */
	FString GetFailureReason() const { return FailureReason; }

	/** Block until complete (for synchronous tests) */
	bool WaitForCompletion();

	/** Tick (non-blocking) */
	void Tick(float DeltaTime);

private:
	bool bIsWaiting;
	bool bWasSuccessful;
	FString FailureReason;
	float RemainingTime;
	float MaxWaitTime;
};

// ============================================================================
// TEST FIXTURE BASE
// ============================================================================

/**
 * Base class for integration test fixtures
 */
class DESKILLZ_API FDeskillzTestFixture
{
public:
	FDeskillzTestFixture();
	virtual ~FDeskillzTestFixture();

	/** Setup test environment */
	virtual void Setup();

	/** Teardown test environment */
	virtual void Teardown();

	/** Initialize SDK with test config */
	void InitializeTestSDK();

	/** Shutdown SDK */
	void ShutdownTestSDK();

	/** Create mock player */
	FDeskillzPlayerInfo CreateTestPlayer();

	/** Create mock tournament */
	FDeskillzTournament CreateTestTournament();

	/** Create mock match */
	FDeskillzMatch CreateTestMatch();

	/** Simulate login */
	bool SimulateLogin(const FString& Username = TEXT("test_user"));

	/** Simulate logout */
	void SimulateLogout();

	/** Check if authenticated */
	bool IsAuthenticated() const { return bIsAuthenticated; }

protected:
	bool bIsSetup;
	bool bIsAuthenticated;
	FString TestUserId;
	FString TestSessionToken;
};

// ============================================================================
// MOCK EXPECTATIONS
// ============================================================================

/**
 * Mock expectation for verifying calls
 */
struct DESKILLZ_API FDeskillzMockExpectation
{
	FString MethodName;
	TArray<FString> ExpectedArgs;
	int32 ExpectedCallCount;
	int32 ActualCallCount;
	bool bShouldFail;
	FString FailureResponse;

	FDeskillzMockExpectation()
		: ExpectedCallCount(1)
		, ActualCallCount(0)
		, bShouldFail(false)
	{}

	bool IsSatisfied() const { return ActualCallCount >= ExpectedCallCount; }
	bool WasOverCalled() const { return ActualCallCount > ExpectedCallCount; }
};

/**
 * Mock expectation manager
 */
class DESKILLZ_API FDeskillzMockExpectationManager
{
public:
	/** Add expectation */
	void Expect(const FString& MethodName, int32 CallCount = 1);

	/** Add expectation with args */
	void ExpectWithArgs(const FString& MethodName, const TArray<FString>& Args);

	/** Add failure expectation */
	void ExpectFailure(const FString& MethodName, const FString& FailureResponse);

	/** Record a call */
	void RecordCall(const FString& MethodName, const TArray<FString>& Args = TArray<FString>());

	/** Verify all expectations */
	bool VerifyAll(FString& OutErrors);

	/** Reset expectations */
	void Reset();

	/** Get expectation for method */
	FDeskillzMockExpectation* GetExpectation(const FString& MethodName);

private:
	TMap<FString, FDeskillzMockExpectation> Expectations;
};
