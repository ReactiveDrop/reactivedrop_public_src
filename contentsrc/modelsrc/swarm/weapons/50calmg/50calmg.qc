$modelname "weapons/50calmg/50calmg.mdl"

$body "default" "50calmg_reference.smd"
$bodygroup "magazine"
{
	studio "50calmg_mag.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/50calmg"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "50calmg_reference.smd"

$collisionmodel "50calmg_physics.smd"
