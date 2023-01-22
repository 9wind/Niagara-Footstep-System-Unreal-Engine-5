// Włodzimierz Iwanowski (c) 2023

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "AC_FootstepSystem.generated.h"

class USoundBase;
class UMaterial;
class UNiagaraSystem; 

USTRUCT(BlueprintType)
struct FFootstepType : public FTableRowBase
{
	GENERATED_BODY()
	
	// Constructor: Sets default values for this struct's variables
	FFootstepType()
	{
		Type = EPhysicalSurface::SurfaceType_Default;
		Sound = nullptr;
		Decal = nullptr;
		Particle = nullptr;
	}

	// EditAnywhere: because we want to modify these in the datatable
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<EPhysicalSurface> Type;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* Sound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterial* Decal;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraSystem* Particle;
};

	
USTRUCT(BlueprintType)
struct FSurfaceInfo
{
	GENERATED_BODY()
	
	// Contains information related to a physical surface
	
	FSurfaceInfo()
	{
		Sound = nullptr;
		Decal = nullptr;
		Particle = nullptr;
	}

	UPROPERTY(BlueprintReadOnly)
	USoundBase* Sound;

	UPROPERTY(BlueprintReadOnly)
	UMaterial* Decal;
	
	UPROPERTY(BlueprintReadOnly)
	UNiagaraSystem* Particle;
};


UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NIAGRAFOOTSTEPSYSTEM_API UAC_FootstepSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAC_FootstepSystem();

	// BlueprintPure: because we want to be able to call this function from blueprint
	// also because these functions have a return type/value ( they return a type/value like: int, float, bool or user-defined types like structs, classes etc... )
	// also these functions won't have execution pins ( only input and output parameters )
	// const: because it doesn't modify any variables in this component
	UFUNCTION(BlueprintPure, Category="Footsteps")
	FSurfaceInfo GetSurfaceInfo(TEnumAsByte<EPhysicalSurface> SurfaceType) const;
	
	UFUNCTION(BlueprintPure, Category="Footsteps")
	FVector GetNextLegLocation(bool bIndex);

	UFUNCTION(BlueprintPure, Category="Footsteps")
	FVector GetLegLocation(bool bIndex) const;

	UFUNCTION(BlueprintPure, Category="Footsteps")
	FRotator GetDecalRotation() const;

	// BlueprintCallable: because we want to be able to call this function from blueprint
	// also this function don't have a return type/value always void ( means doesn't return a value )
	// also this function will have execution pins
	UFUNCTION(BlueprintCallable, Category="Footsteps")
	void LegEvent();
	

	// EditDefaultsOnly: because we want to edit these variables in blueprints defaults tab only. (EditAnywhere can be used also if you want to edit variables anywhere)
	// BlueprintReadOnly: because we want the blueprint to be able to read these variables without modifying them.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Footsteps")
	UDataTable* StepsDatatable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Footsteps")
	FName LeftFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Footsteps")
	FName RightFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Footsteps")
	bool bLeftLeg;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Footsteps")
	float DecalLifeSpan;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Footsteps")
	FVector DecalSize;

};
