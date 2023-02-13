$modelname "weapons/Pistol/pistol.mdl"

$bodygroup "magazine"
{
	studio "pistol_model_loaded.smd"
	studio "pistol_model_unloaded.smd"
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/Pistol"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject" "eject" 0 0 0 rotate 0 0 0
$attachment "eject2" "eject2" 0 0 0 rotate 0 0 0
$attachment "muzzle_flash_l" "muzzle_flash_l" 0 0 0 rotate 0 0 0

$sequence "BindPose" "pistol_model_loaded.smd"

$collisionmodel "pistol_physics.smd" { $concave }
