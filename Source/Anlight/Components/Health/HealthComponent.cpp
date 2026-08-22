// HealthComponent.cpp
#include "Components/Health/HealthComponent.h"
#include "Components/EffectManager/EffectManager.h"
#include "Core/FAnlightDamageEvent.h"
#include "Net/UnrealNetwork.h"

// ============================================================
// КОНСТРУКТОР И ИНИЦИАЛИЗАЦИЯ
// ============================================================

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    SetIsReplicatedByDefault(true);

    BodyStates.SetNum(static_cast<int32>(EBodyPart::MAX));

    for (int32 i = 0; i < BodyStates.Num(); ++i)
    {
        BodyStates[i].Part = static_cast<EBodyPart>(i);
    }
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GetOwner()->HasAuthority())
    {
        InitializeHealth();
    }
}

void UHealthComponent::InitializeHealth()
{
    for (int32 i = 0; i < BodyStates.Num(); ++i)
    {
        EBodyPart Part = static_cast<EBodyPart>(i);
        BodyStates[i].Initialize(Part);
    }

    PainData.CurrentBloodLoss = 0.f;
    PainData.CurrentToxicity = 0.f;

    UpdateHealthUI();
    UpdateAllPartPenalties();
}

// ============================================================
// TICK
// ============================================================

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // if (!GetOwner()->HasAuthority())
    //     return;

    // 1. Кровотечения
    ApplyBleedingDamage(DeltaTime);

    // 2. Регенерация
    ApplyRegeneration(DeltaTime);

    // 3. Токсины
    ApplyToxinReduction(DeltaTime);

    // 4. Проверка смерти
    CheckDeathConditions();

    // 5. UI
    UpdateHealthUI();
}

// ============================================================
// НАНЕСЕНИЕ УРОНА
// ============================================================

float UHealthComponent::ApplyDamageToPart(EBodyPart Part, float Damage, const FAnlightDamageEvent& DamageEvent)
{
    if (!GetOwner()->HasAuthority())
        return 0.f;

    if (Part == EBodyPart::None || Part >= EBodyPart::MAX)
        return 0.f;

    int32 Index = GetPartIndex(Part);
    if (Index == INDEX_NONE)
        return 0.f;

    float DamageMultiplier = 1.f;
    if (EffectManager)
    {
        DamageMultiplier += EffectManager->GetTotalModifier(EAnlightStat::IncomingDamageModifier);
        DamageMultiplier += EffectManager->GetTotalModifierForPart(EAnlightStat::IncomingDamageModifier, Part);
    }
    DamageMultiplier = FMath::Max(0.1f, DamageMultiplier);

    float FinalDamage = Damage * DamageMultiplier;
    float RemainingDamage = BodyStates[Index].ApplyDamage(FinalDamage);

    if (FinalDamage > 15.f)
    {
        AddBloodLoss(FinalDamage * 0.1f);
    }

    if (BodyStates[Index].bIsDestroyed)
    {
        UE_LOG(LogTemp, Warning, TEXT("Body part destroyed: %s"), *UEnum::GetValueAsString(Part));
        OnBodyPartDestroyed.Broadcast(Part);
        UpdatePartPenalties(Part, true);

        if (Part == EBodyPart::Head)
        {
            UE_LOG(LogTemp, Error, TEXT("HEAD DESTROYED - broadcasting death!"));
            OnHealthDepleted.Broadcast();
            return FinalDamage;
        }
    }

    if (RemainingDamage > 0.f)
    {
        SpreadRemainingDamage(Part, RemainingDamage, DamageEvent);
    }

    OnBodyPartHealthChanged.Broadcast(Part, BodyStates[Index].CurrentHealth);
    UpdateHealthUI();

    return FinalDamage - RemainingDamage;
}

// ============================================================
// РАСПРЕДЕЛЕНИЕ OVERKILL
// ============================================================

void UHealthComponent::SpreadRemainingDamage(EBodyPart SourcePart, float RemainingDamage, const FAnlightDamageEvent& DamageEvent)
{
    TArray<EBodyPart> TargetParts;
    float DamageMultiplier = 1.f;

    switch (SourcePart)
    {
    case EBodyPart::Head:
        return;

    case EBodyPart::Chest:
        for (int32 i = 0; i < BodyStates.Num(); ++i)
        {
            EBodyPart Part = static_cast<EBodyPart>(i);
            if (Part != EBodyPart::None && Part != SourcePart && BodyStates[i].IsAlive())
            {
                TargetParts.Add(Part);
            }
        }
        DamageMultiplier = 1.5f;
        break;

    case EBodyPart::LeftArm:
    case EBodyPart::RightArm:
    case EBodyPart::LeftLeg:
    case EBodyPart::RightLeg:
        if (GetBodyPartState(EBodyPart::Chest).IsAlive())
        {
            TargetParts.Add(EBodyPart::Chest);
            DamageMultiplier = 0.7f;
        }
        else
        {
            for (int32 i = 0; i < BodyStates.Num(); ++i)
            {
                EBodyPart Part = static_cast<EBodyPart>(i);
                if (Part != EBodyPart::None && Part != SourcePart && BodyStates[i].IsAlive())
                {
                    TargetParts.Add(Part);
                }
            }
            DamageMultiplier = 1.2f;
        }
        break;

    default:
        return;
    }

    if (TargetParts.Num() == 0)
        return;

    float DamagePerPart = (RemainingDamage * DamageMultiplier) / TargetParts.Num();

    for (EBodyPart TargetPart : TargetParts)
    {
        int32 TargetIndex = GetPartIndex(TargetPart);
        if (TargetIndex != INDEX_NONE && BodyStates[TargetIndex].IsAlive())
        {
            FAnlightDamageEvent NewEvent = DamageEvent;
            NewEvent.HitBodyPart = TargetPart;

            float Overkill = BodyStates[TargetIndex].ApplyDamage(DamagePerPart);

            if (BodyStates[TargetIndex].bIsDestroyed)
            {
                OnBodyPartDestroyed.Broadcast(TargetPart);
                UpdatePartPenalties(TargetPart, true);

                if (TargetPart == EBodyPart::Head)
                {
                    UE_LOG(LogTemp, Error, TEXT("HEAD DESTROYED (overkill) - broadcasting death!"));
                    OnHealthDepleted.Broadcast();
                    return;
                }
            }

            if (Overkill > 0.f)
            {
                SpreadRemainingDamage(TargetPart, Overkill, NewEvent);
            }
        }
    }
}

// ============================================================
// ЛЕЧЕНИЕ И ХИРУРГИЯ
// ============================================================

void UHealthComponent::HealPart(EBodyPart Part, float Amount)
{
    if (!GetOwner()->HasAuthority())
        return;

    int32 Index = GetPartIndex(Part);
    if (Index == INDEX_NONE)
        return;

    float HealMultiplier = 1.f;
    if (EffectManager)
    {
        HealMultiplier += EffectManager->GetTotalModifier(EAnlightStat::HealingEfficiency);
    }
    HealMultiplier = FMath::Max(0.1f, HealMultiplier);

    BodyStates[Index].Heal(Amount * HealMultiplier);
    OnBodyPartHealthChanged.Broadcast(Part, BodyStates[Index].CurrentHealth);
    UpdateHealthUI();
}

void UHealthComponent::PerformSurgery(EBodyPart Part, float RestoredHealth)
{
    if (!GetOwner()->HasAuthority())
        return;

    int32 Index = GetPartIndex(Part);
    if (Index == INDEX_NONE)
        return;

    if (BodyStates[Index].bIsDestroyed)
    {
        BodyStates[Index].PerformSurgery(RestoredHealth);
        UpdatePartPenalties(Part, false);
        OnBodyPartHealthChanged.Broadcast(Part, BodyStates[Index].CurrentHealth);
        UpdateHealthUI();
    }
}

// ============================================================
// КРОВОТЕЧЕНИЕ
// ============================================================

void UHealthComponent::StartBleeding(EBodyPart Part, float Intensity)
{
    if (!GetOwner()->HasAuthority())
        return;

    int32 Index = GetPartIndex(Part);
    if (Index == INDEX_NONE)
        return;

    if (EffectManager)
    {
        float Resistance = EffectManager->GetTotalModifier(EAnlightStat::BleedingResistance);
        if (FMath::FRand() < Resistance)
            return;
    }

    BodyStates[Index].StartBleeding(Intensity);

    UE_LOG(LogTemp, Warning, TEXT("Bleeding started: %s, bIsBleeding=%s, Intensity=%f"),
        *UEnum::GetValueAsString(Part),
        BodyStates[Index].bIsBleeding ? TEXT("TRUE") : TEXT("FALSE"),
        Intensity);

    UpdateHealthUI();
}

void UHealthComponent::StopBleeding(EBodyPart Part)
{
    if (!GetOwner()->HasAuthority())
        return;

    int32 Index = GetPartIndex(Part);
    if (Index == INDEX_NONE)
        return;

    BodyStates[Index].StopBleeding();
    UpdateHealthUI();
}

void UHealthComponent::StopAllBleeding()
{
    if (!GetOwner()->HasAuthority())
        return;

    for (FBodyPartState& State : BodyStates)
    {
        State.StopBleeding();
    }
    UpdateHealthUI();
}

bool UHealthComponent::HasBleedingOnPart(EBodyPart Part) const
{
    int32 Index = GetPartIndex(Part);
    if (Index != INDEX_NONE)
        return BodyStates[Index].bIsBleeding;
    return false;
}

// ============================================================
// КРОВОПОТЕРЯ
// ============================================================

void UHealthComponent::AddBloodLoss(float Amount)
{
    if (!GetOwner()->HasAuthority())
        return;

    PainData.AddBloodLoss(Amount);
    OnBloodLossChanged.Broadcast(PainData.CurrentBloodLoss, PainData.MaxBloodLoss);
}

// ============================================================
// ИНТОКСИКАЦИЯ
// ============================================================

void UHealthComponent::AddToxicity(float Amount)
{
    /*if (!GetOwner()->HasAuthority())
        return;*/

    if (EffectManager)
    {
        float Resistance = EffectManager->GetTotalModifier(EAnlightStat::ToxinResistance);
        Amount = FMath::Max(0.f, Amount - Resistance);
    }

    if (Amount > 0.f)
    {
        PainData.AddToxicity(Amount);
        OnToxicityChanged.Broadcast(PainData.CurrentToxicity, PainData.MaxToxicity);
        UpdateToxicityVisual();

        // НЕМЕДЛЕННАЯ ПРОВЕРКА СМЕРТИ
        if (PainData.IsDeadFromToxicity())
        {
            UE_LOG(LogTemp, Error, TEXT("DEATH FROM TOXICITY! Broadcasting OnHealthDepleted"));
            OnHealthDepleted.Broadcast();
        }
    }
}

void UHealthComponent::ReduceToxicity(float Amount)
{
    /*if (!GetOwner()->HasAuthority())
        return;*/

    PainData.ReduceToxicity(Amount);
    OnToxicityChanged.Broadcast(PainData.CurrentToxicity, PainData.MaxToxicity);
    UpdateToxicityVisual();
}

//=====================================================
// М Е Д И Ц И Н А
//=====================================================

// Аптечка EDC
void UHealthComponent::UseMedkitEDC(float HealAmount)
{
    /*if (!GetOwner()->HasAuthority())
        return;*/

    // сбор зон, что можно вылечить
    TArray<EBodyPart> HealableParts;
    for (const FBodyPartState& State : BodyStates)
    {
        if (State.IsAlive() && State.CurrentHealth < State.MaxHealth)
        {
            HealableParts.Add(State.Part);
        }
    }

    // если таких нет - ничего не делаем
    if (HealableParts.Num() == 0)
        return;

    // выбираем рандомную
    int32 RandomIndex = FMath::RandRange(0, HealableParts.Num() - 1);
    HealPart(HealableParts[RandomIndex], HealAmount);
}

// Эсмарх (не Турникет блять)
void UHealthComponent::UseEsmarch(float BleedReduction)
{
    /*if (!GetOwner()->HasAuthority())
        return;*/

    // Собираем кровоточащие зоны
    TArray<EBodyPart> BleedingParts;
    for (const FBodyPartState& State : BodyStates)
    {
        if (State.bIsBleeding)
        {
            BleedingParts.Add(State.Part);
        }
    }
    // Если кровотечений нет — ничего не делаем
    if (BleedingParts.Num() == 0)
        return;

    // Выбираем случайную
    int32 RandomIndex = FMath::RandRange(0, BleedingParts.Num() - 1);
    StopBleeding(BleedingParts[RandomIndex]);
}

// Турникет - временно как эсмарх
void UHealthComponent::UseTourniquet(float BleedReduction)
{
    /*if (!GetOwner()->HasAuthority())
        return;*/

    // Собираем кровоточащие зоны
    TArray<EBodyPart> BleedingParts;
    for (const FBodyPartState& State : BodyStates)
    {
        if (State.bIsBleeding)
        {
            BleedingParts.Add(State.Part);
        }
    }
    // Если кровотечений нет — ничего не делаем
    if (BleedingParts.Num() == 0)
        return;

    // Выбираем случайную
    int32 RandomIndex = FMath::RandRange(0, BleedingParts.Num() - 1);
    StopBleeding(BleedingParts[RandomIndex]);
}

// Универсальное обезболивающее
void UHealthComponent::UsePainkiller(float Duration, float ToxicityAmount)
{
    if(EffectManager && EffectManager->HasEffect(FName("Antidote")))
    {
        UE_LOG(LogTemp, Warning, TEXT("Painkiller blocked: antidote active"));
        return;
    }

    ApplyPainkiller(Duration);

    if (ToxicityAmount > 0.0f)
    {
        AddToxicity(ToxicityAmount);
    }
}

// Универсальный антидот
void UHealthComponent::UseAntidote(float ToxicityReduction, float Duration)
{
    if (!EffectManager) return;

    FStatusEffect AntidoteEffect;
    AntidoteEffect.EffectID = FName("Antidote");
    AntidoteEffect.AffectedStat = EAnlightStat::ToxinReductionRate;
    AntidoteEffect.Value = ToxicityReduction / Duration; // сколько в секунду
    AntidoteEffect.Duration = Duration;
    AntidoteEffect.TickInterval = 1.0f;
    EffectManager->ApplyEffect(AntidoteEffect);
}





// ============================================================
// ГЕТТЕРЫ
// ============================================================

FBodyPartState UHealthComponent::GetBodyPartState(EBodyPart Part) const
{
    int32 Index = GetPartIndex(Part);
    if (Index != INDEX_NONE)
        return BodyStates[Index];
    return FBodyPartState();
}

float UHealthComponent::GetTotalHealth() const
{
    float Total = 0.f;
    for (const FBodyPartState& State : BodyStates)
    {
        Total += State.CurrentHealth;
    }
    return Total;
}

float UHealthComponent::GetTotalMaxHealth() const
{
    float Total = 0.f;
    for (const FBodyPartState& State : BodyStates)
    {
        Total += State.MaxHealth;
    }
    return Total;
}

bool UHealthComponent::IsDead() const
{
    if (GetBodyPartState(EBodyPart::Head).bIsDestroyed)
        return true;

    if (PainData.IsDeadFromBloodLoss())
        return true;

    if (PainData.IsDeadFromToxicity())
        return true;

    return false;
}

bool UHealthComponent::IsPartDestroyed(EBodyPart Part) const
{
    return GetBodyPartState(Part).bIsDestroyed;
}

// ============================================================
// ВНУТРЕННИЕ МЕТОДЫ
// ============================================================

void UHealthComponent::ApplyBleedingDamage(float DeltaTime)
{
    for (FBodyPartState& State : BodyStates)
    {
        if (State.bIsBleeding && State.BleedingIntensity > 0.f)
        {
            float BleedDamage = State.GetBleedingRate() * DeltaTime;
            if (BleedDamage > 0.f)
            {
                UE_LOG(LogTemp, Warning, TEXT("Applying bleed damage to %s"),
                    *UEnum::GetValueAsString(State.Part));

                State.CurrentHealth = FMath::Max(0.f, State.CurrentHealth - BleedDamage);
                OnBodyPartHealthChanged.Broadcast(State.Part, State.CurrentHealth);
                
                UpdateHealthUI();

                if (State.CurrentHealth <= 0.f && !State.bIsDestroyed)
                {
                    State.bIsDestroyed = true;
                    State.CurrentHealth = 0.f;
                    OnBodyPartDestroyed.Broadcast(State.Part);
                    UpdatePartPenalties(State.Part, true);

                    if (State.Part == EBodyPart::Head)
                    {
                        UE_LOG(LogTemp, Error, TEXT("HEAD DESTROYED (bleeding) - broadcasting death!"));
                        OnHealthDepleted.Broadcast();
                        return;
                    }
                }

                AddBloodLoss(BleedDamage * 0.05f);
            }
        }
    }
}

void UHealthComponent::ApplyRegeneration(float DeltaTime)
{
    for (FBodyPartState& State : BodyStates)
    {
        if (State.IsAlive() && !State.bIsBleeding)
        {
            float NewHealth = State.CurrentHealth + HealthRegenRate * DeltaTime;
            State.CurrentHealth = FMath::Min(NewHealth, State.MaxHealth);
        }
    }
}

void UHealthComponent::ApplyToxinReduction(float DeltaTime)
{
    float Reduction = PainData.NaturalToxinReductionRate * DeltaTime;

    if (EffectManager)
    {
        Reduction += EffectManager->GetTotalModifier(EAnlightStat::ToxinReductionRate) * DeltaTime;
    }

    if (Reduction > 0.f && PainData.CurrentToxicity > 0.f)
    {
        PainData.ReduceToxicity(Reduction);
        OnToxicityChanged.Broadcast(PainData.CurrentToxicity, PainData.MaxToxicity);
        UpdateToxicityVisual();
    }
}

void UHealthComponent::CheckDeathConditions()
{
    if (IsDead())
    {
        OnHealthDepleted.Broadcast();
    }
}

void UHealthComponent::UpdatePartPenalties(EBodyPart Part, bool bIsDestroyed)
{
    if (EffectManager && EffectManager->HasEffect(FName("Painkiller")))
    {
        UE_LOG(LogTemp, Warning, TEXT("Painkiller active - skipping penalties"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("UpdatePartPenalties called: %s, Destroyed=%s"),
        *UEnum::GetValueAsString(Part),
        bIsDestroyed ? TEXT("YES") : TEXT("NO"));

    if (!EffectManager)
    {
        UE_LOG(LogTemp, Error, TEXT("EffectManager is nullptr!"));
        return;
    }

    if (Part == EBodyPart::Chest)
    {
        if (bIsDestroyed)
        {
            FStatusEffect StaminaEffect;
            StaminaEffect.EffectID = FName("Chest_Stamina_Penalty");
            StaminaEffect.AffectedStat = EAnlightStat::StaminaMultiplier;
            StaminaEffect.Value = 0.05f;
            StaminaEffect.Duration = 0.f;
            StaminaEffect.TargetBodyPart = EBodyPart::Chest;
            EffectManager->ApplyEffect(StaminaEffect);

            FStatusEffect StaminaRegenEffect;
            StaminaRegenEffect.EffectID = FName("Chest_StaminaRegen_Penalty");
            StaminaRegenEffect.AffectedStat = EAnlightStat::StaminaRegenMultiplier;
            StaminaRegenEffect.Value = 0.5f;
            StaminaRegenEffect.Duration = 0.f;
            StaminaRegenEffect.TargetBodyPart = EBodyPart::Chest;
            EffectManager->ApplyEffect(StaminaRegenEffect);
        }
        else
        {
            EffectManager->RemoveEffect(FName("Chest_Stamina_Penalty"), EBodyPart::Chest);
            EffectManager->RemoveEffect(FName("Chest_StaminaRegen_Penalty"), EBodyPart::Chest);
        }
        return;
    }

    if (Part == EBodyPart::LeftArm || Part == EBodyPart::RightArm)
    {
        bool bLeftDestroyed = GetBodyPartState(EBodyPart::LeftArm).bIsDestroyed;
        bool bRightDestroyed = GetBodyPartState(EBodyPart::RightArm).bIsDestroyed;
        bool bAnyDestroyed = bLeftDestroyed || bRightDestroyed;

        if (bAnyDestroyed)
        {
            FStatusEffect SpreadEffect;
            SpreadEffect.EffectID = FName("Arms_Spread_Penalty");
            SpreadEffect.AffectedStat = EAnlightStat::WeaponSpreadMultiplier;
            SpreadEffect.Value = 1.3f;
            SpreadEffect.Duration = 0.f;
            SpreadEffect.TargetBodyPart = EBodyPart::None;
            EffectManager->ApplyEffect(SpreadEffect);

            FStatusEffect AnimEffect;
            AnimEffect.EffectID = FName("Arms_AnimSpeed_Penalty");
            AnimEffect.AffectedStat = EAnlightStat::AnimationSpeedMultiplier;
            AnimEffect.Value = 0.35f;
            AnimEffect.Duration = 0.f;
            AnimEffect.TargetBodyPart = EBodyPart::None;
            EffectManager->ApplyEffect(AnimEffect);
        }
        else
        {
            EffectManager->RemoveEffect(FName("Arms_Spread_Penalty"), EBodyPart::None);
            EffectManager->RemoveEffect(FName("Arms_AnimSpeed_Penalty"), EBodyPart::None);
        }
        return;
    }

    if (Part == EBodyPart::LeftLeg || Part == EBodyPart::RightLeg)
    {
        bool bLeftDestroyed = GetBodyPartState(EBodyPart::LeftLeg).bIsDestroyed;
        bool bRightDestroyed = GetBodyPartState(EBodyPart::RightLeg).bIsDestroyed;

        float SpeedMult = 1.f;
        if (bLeftDestroyed) SpeedMult *= 0.125f;
        if (bRightDestroyed) SpeedMult *= 0.125f;

        if (SpeedMult < 1.f)
        {
            FStatusEffect SpeedEffect;
            SpeedEffect.EffectID = FName("Legs_Speed_Penalty");
            SpeedEffect.AffectedStat = EAnlightStat::MovementSpeedMultiplier;
            SpeedEffect.Value = SpeedMult;
            SpeedEffect.Duration = 0.f;
            SpeedEffect.TargetBodyPart = EBodyPart::None;
            EffectManager->ApplyEffect(SpeedEffect);
        }
        else
        {
            EffectManager->RemoveEffect(FName("Legs_Speed_Penalty"), EBodyPart::None);
        }

        bool bCanJump = !(bLeftDestroyed && bRightDestroyed);

        if (!bCanJump)
        {
            FStatusEffect JumpEffect;
            JumpEffect.EffectID = FName("Legs_Jump_Penalty");
            JumpEffect.AffectedStat = EAnlightStat::bCanJump;
            JumpEffect.Value = 0.f;
            JumpEffect.Duration = 0.f;
            JumpEffect.TargetBodyPart = EBodyPart::None;
            EffectManager->ApplyEffect(JumpEffect);
        }
        else
        {
            EffectManager->RemoveEffect(FName("Legs_Jump_Penalty"), EBodyPart::None);
        }
        return;
    }
}

void UHealthComponent::UpdateAllPartPenalties()
{
    if (!EffectManager)
        return;

    EffectManager->RemoveEffect(FName("Legs_Speed_Penalty"));
    EffectManager->RemoveEffect(FName("Legs_Jump_Penalty"));
    EffectManager->RemoveEffect(FName("Arms_Spread_Penalty"));
    EffectManager->RemoveEffect(FName("Arms_AnimSpeed_Penalty"));
    EffectManager->RemoveEffect(FName("Chest_Stamina_Penalty"));
    EffectManager->RemoveEffect(FName("Chest_StaminaRegen_Penalty"));

    for (const FBodyPartState& State : BodyStates)
    {
        if (State.bIsDestroyed)
        {
            UpdatePartPenalties(State.Part, true);
        }
    }
}

void UHealthComponent::ApplyPainkiller(float Duration)
{
    if (!EffectManager)
        return;

    EffectManager->RemoveEffect(FName("Legs_Speed_Penalty"));
    EffectManager->RemoveEffect(FName("Legs_Jump_Penalty"));
    EffectManager->RemoveEffect(FName("Arms_Spread_Penalty"));
    EffectManager->RemoveEffect(FName("Arms_AnimSpeed_Penalty"));
    EffectManager->RemoveEffect(FName("Chest_Stamina_Penalty"));
    EffectManager->RemoveEffect(FName("Chest_StaminaRegen_Penalty"));

    FStatusEffect PainkillerEffect;
    PainkillerEffect.EffectID = FName("Painkiller");
    PainkillerEffect.AffectedStat = EAnlightStat::Health;
    PainkillerEffect.Value = 0.0f;
    PainkillerEffect.Duration = Duration;
    EffectManager->ApplyEffect(PainkillerEffect);
}

void UHealthComponent::RemovePainkiller()
{
    if (!EffectManager)
        return;

    EffectManager->RemoveEffect(FName("Painkiller"));
    UpdateAllPartPenalties();
}

void UHealthComponent::OnPainkillerExpired(const FStatusEffect& Effect)
{
    if (Effect.EffectID == FName("Painkiller"))
    {
        UpdateAllPartPenalties();
    }
}

void UHealthComponent::UpdateHealthUI()
{
    OnHealthChanged.Broadcast(GetTotalHealth(), GetTotalMaxHealth());

    bool bAnyBleeding = false;
    for (const FBodyPartState& State : BodyStates)
    {
        if (State.bIsBleeding)
        {
            bAnyBleeding = true;
            break;
        }
    }
    OnBleedingChanged.Broadcast(bAnyBleeding);
}

int32 UHealthComponent::GetPartIndex(EBodyPart Part) const
{
    int32 Index = static_cast<int32>(Part);
    if (Index >= 0 && Index < BodyStates.Num())
        return Index;
    return INDEX_NONE;
}

bool UHealthComponent::IsVitalPart(EBodyPart Part) const
{
    return Part == EBodyPart::Head || Part == EBodyPart::Chest;
}

void UHealthComponent::SetEffectManager(UEffectManager* InEffectManager)
{
    EffectManager = InEffectManager;

    if (EffectManager)
    {
        EffectManager->OnEffectRemoved.AddDynamic(this, &UHealthComponent::OnPainkillerExpired);
    }
}

void UHealthComponent::UpdateToxicityVisual()
{
    const float Toxicity = PainData.CurrentToxicity;

    // До 400 — обычный цвет
    if (Toxicity < 400.0f)
    {
        OnToxicityVisualChanged.Broadcast(FLinearColor::White);
        OnCriticalToxicity.Broadcast(false);
        return;
    }

    // От 400 до 700 — плавный переход к бледно-зелёному
    const float Alpha = FMath::Clamp((Toxicity - 400.0f) / 300.0f, 0.0f, 1.0f);
    const FLinearColor NormalColor = FLinearColor::White;
    const FLinearColor ToxicColor = FLinearColor(0.65f, 0.85f, 0.65f, 1.0f); // бледно-зелёный
    const FLinearColor Result = FMath::Lerp(NormalColor, ToxicColor, Alpha);

    OnToxicityVisualChanged.Broadcast(Result);

    // Критическая иконка при 700+
    OnCriticalToxicity.Broadcast(Toxicity >= 700.0f);
}



// ============================================================
// РЕПЛИКАЦИЯ
// ============================================================

void UHealthComponent::OnRep_BodyStates()
{
    UpdateHealthUI();
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(UHealthComponent, BodyStates, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(UHealthComponent, PainData, COND_OwnerOnly);
}