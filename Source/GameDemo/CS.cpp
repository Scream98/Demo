// Fill out your copyright notice in the Description page of Project Settings.


#include "CS.h"

// Sets default values
ACS::ACS()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACS::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACS::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACS::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

