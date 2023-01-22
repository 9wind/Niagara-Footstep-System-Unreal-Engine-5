// Włodzimierz Iwanowski (c) 2023


#include "AC_FootstepSystem.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values for this component's properties
UAC_FootstepSystem::UAC_FootstepSystem()
{
	LeftFootSocketName = TEXT("foot_l");
	RightFootSocketName = TEXT("foot_r");

	bLeftLeg = false;

	DecalLifeSpan = 10.0f;
	DecalSize = FVector(200.0f, 32.0f, 6.0f);
}

FSurfaceInfo UAC_FootstepSystem::GetSurfaceInfo(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	// check if there's a datatable selected in the footsteps component blueprint
	if (StepsDatatable == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetSurfaceInfo: Invalid Datatable in FootstepsComponent."));
		return FSurfaceInfo();
	}

	// retrieve row names from the datatable
	TArray<FName> RowNames = StepsDatatable->GetRowNames();

	for (const FName& RowName: RowNames)
	{
		// Search for row by name
		const FFootstepType* RowData = StepsDatatable->FindRow<FFootstepType>(RowName, TEXT(""), /*bWarnIfRowMissing=*/ false);

		// If nothing found, skip and search for the next row.
		if (RowData == nullptr)
		{
			continue; 
		}

		// If the row we found has the same surface type we looking for, return row data
		if (RowData->Type == SurfaceType)
		{
			FSurfaceInfo Result;
			Result.Sound = RowData->Sound;
			Result.Decal = RowData->Decal;
			Result.Particle = RowData->Particle;

			return Result;
		}
	}

	// if the surface type we looking for is not found, return the default surface info
	return GetSurfaceInfo(SurfaceType_Default);
}

FVector UAC_FootstepSystem::GetNextLegLocation(bool bIndex)
{
	bLeftLeg = !bLeftLeg;	// LeftLeg = (Not)LeftLeg : flips boolean value
	return GetLegLocation(bIndex);	// return the leg location after flipping the Index boolean.
}

FVector UAC_FootstepSystem::GetLegLocation(bool bIndex) const
{
	// Get the component owner and cast it to ACharacter class
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	
	check(Character != nullptr);	// If Owner or Character are nullptr (invalid) this will crash the editor
	// And it means the component has no owner, or the owner is not a character

	if (bIndex)
	{
		// If Index is true, get the left foot socket location
		return Character->GetMesh()->GetSocketLocation(LeftFootSocketName);
	}
	else
	{
		// If Index is false, get the right foot socket location
		return Character->GetMesh()->GetSocketLocation(RightFootSocketName);
	}
}

FRotator UAC_FootstepSystem::GetDecalRotation() const
{
	// Get the component owner actor rotation
	FRotator OwnerActorRotation = GetOwner()->GetActorRotation();

	// decrease pith value by 90.0f
	OwnerActorRotation.Pitch -= 90.0f;

	// decrease yaw value by -90.0f
	OwnerActorRotation.Yaw -= -90.0f;

	// return the modified rotator
	return OwnerActorRotation;
}

void UAC_FootstepSystem::LegEvent()
{
	// Get the leg location and store it
	const FVector LegLocation = GetNextLegLocation(bLeftLeg);

	// store line trace starting location
	FVector TraceStartPoint = LegLocation;

	// store line trace ending location
	FVector TraceEndPoint = LegLocation;
	TraceEndPoint.Z = -5000.0f;

	// Ignore the actor who owns this component if it gets hit by the line trace.
	// If we hit something include its physical material in the hit result.
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bReturnPhysicalMaterial = true;

	// Initialize hit result data
	FHitResult HitResult (ForceInit);
	// perform a line trace by channel
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStartPoint, TraceEndPoint, ECollisionChannel::ECC_Visibility, QueryParams);

	// if we hit something and a physical material was included in the hit result
	if (HitResult.PhysMaterial.Get() != nullptr)
	{
		// get the surface info from that physical material surface type we hit
		FSurfaceInfo SurfaceInfo = GetSurfaceInfo(HitResult.PhysMaterial.Get()->SurfaceType);

		// if there's a sound in the surface info for that surface type, play it at component owner actor location
		if (SurfaceInfo.Sound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, SurfaceInfo.Sound, GetOwner()->GetActorLocation());
		}

		// if there's a decal material in the surface info for that surface type, spawn that decal at hit location
		// Also applies decal size, decal rotation, lifespan etc..
		if (SurfaceInfo.Decal != nullptr)
		{
			UGameplayStatics::SpawnDecalAtLocation(this, SurfaceInfo.Decal, DecalSize, HitResult.Location, GetDecalRotation(), DecalLifeSpan);
		}

		// if there's a particle effect in the surface info for that surface type, spawn and play that particle effect at the specified location.
		if (SurfaceInfo.Particle != nullptr)
		{
			// Calculate ActorForwardVector x Velocity vector length ÷ (divided by) 3.0 
			FVector ActorForwardVector = GetOwner()->GetActorForwardVector() * (GetOwner()->GetVelocity().Size() / 3.0f);

			// Modify the Z axis of the Vector result to 0.0
			ActorForwardVector.Z = 0.0f;

			// Calculate LineTrace Hit location + ( ActorForwardVector x Velocity vector length ÷ (divided by) 3.0 )
			FVector HitLocationPlusForwardVector = HitResult.Location + ActorForwardVector;

			// Calculate final particle effect spawn location ( HitLocationPlusForwardVector + Vector(0.0, 0.0, 7.0) )
			FVector EmitterLocation = HitLocationPlusForwardVector + FVector(0.0f, 0.0f, 7.0f);

			// Spawn particle effect at that location (EmitterLocation), with zero rotation (0.0, 0.0, 0.0) and scale of (1.0, 1.0, 1.0)
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SurfaceInfo.Particle, EmitterLocation); 
		}
	}
}

