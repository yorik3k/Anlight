
#include "gamededPrjCharacter.h"

#include "HealthComponent.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SprintComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AgamededPrjCharacter

AgamededPrjCharacter::AgamededPrjCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
    FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
    FirstPersonCameraComponent->bUsePawnControlRotation = true;

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
    SprintComponent = CreateDefaultSubobject<USprintComponent>(TEXT("SprintComponent"));

    Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
    Mesh1P->SetOnlyOwnerSee(true);
    Mesh1P->SetupAttachment(FirstPersonCameraComponent);
    Mesh1P->bCastDynamicShadow = false;
    Mesh1P->CastShadow = false;
    Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

    characterDye = false;
}

void AgamededPrjCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void AgamededPrjCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AgamededPrjCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AgamededPrjCharacter::Move);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AgamededPrjCharacter::Look);

        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AgamededPrjCharacter::StartSprint);
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AgamededPrjCharacter::StopSprint);
    }
    else
    {
        UE_LOG(LogTemplateCharacter, Error, TEXT("Enhanced Input Component not found!"));
    }
}

void AgamededPrjCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        AddMovementInput(GetActorForwardVector(), MovementVector.Y);
        AddMovementInput(GetActorRightVector(), MovementVector.X);
    }
}

void AgamededPrjCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void AgamededPrjCharacter::dye()
{
    if (characterDye) return;

    characterDye = true;

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->DisableInput(PC);
    }

    FirstPersonCameraComponent->bUsePawnControlRotation = false;
    GetCharacterMovement()->DisableMovement();

    GetWorld()->GetTimerManager().SetTimer(DeathTimerHandle, this, &AgamededPrjCharacter::UpdateDeathAnimation, 0.02f, true);
}

void AgamededPrjCharacter::UpdateDeathAnimation()
{
    static float elaspedTime = 0.0f;
    elaspedTime += 0.02f;

    float duration = 1.5f;
    float alpha = FMath::Min(elaspedTime / duration, 1.0f);
    float EasedAlpha = alpha * alpha;

    FVector currentLocation = FirstPersonCameraComponent->GetRelativeLocation();
    float TargetZ = FMath::Lerp(0.0f, deathLocation, EasedAlpha);
    FirstPersonCameraComponent->SetRelativeLocation(FVector(currentLocation.X, currentLocation.Y, TargetZ));

    float TargetPitch = FMath::Lerp(0.0f, deathPitchAngle, EasedAlpha);
    FRotator TargetRotation = FRotator(TargetPitch, 0.0f, 0.0f);
    FirstPersonCameraComponent->SetRelativeRotation(TargetRotation);

    if (alpha >= 1.0f)
    {
        GetWorld()->GetTimerManager().ClearTimer(DeathTimerHandle);
        elaspedTime = 0.0f;
        FirstPersonCameraComponent->bUsePawnControlRotation = true;
        OnDeathSequenceFinished();
    }
}
void AgamededPrjCharacter::StartSprint()
{
    if (characterDye) return;
    if (SprintComponent) SprintComponent->StartSprint();
}

void AgamededPrjCharacter::StopSprint()
{
    if (SprintComponent) SprintComponent->StopSprint();
}