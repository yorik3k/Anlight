// FAnlightDamageEvent.h
#pragma once

#include "CoreMinimal.h"
#include "EBodyPart.h"
#include "FAnlightDamageEvent.generated.h"

/**
 * Информация о нанесенном уроне по частям тела
 */
USTRUCT(BlueprintType)
struct ANLIGHT_API FAnlightDamageEvent
{
    GENERATED_BODY()

public:
    // Часть тела, в которую попали
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBodyPart HitBodyPart = EBodyPart::None;

    // Базовый урон (до модификаторов)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDamage = 0.f;

    // Сила пробития
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PenetrationPower = 0.f;

    // Тип урона (пуля, взрыв, падение)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName DamageType = "Default";

    // Прошёл ли урон сквозь броню
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPenetratedArmor = false;
};