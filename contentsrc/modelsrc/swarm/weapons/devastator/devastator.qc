// Created by Crowbar 0.72

$modelname "weapons/Devastator/Devastator.mdl"

$body "default" "devastator_reference.smd"

$bodygroup "magazine"
{
	studio "devastator_mag.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/Devastator"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "devastator_reference.smd"
$sequence "pull_bolt" {
	"anims/pull_bolt.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
}

$collisionmodel "devastator_physics.smd"
