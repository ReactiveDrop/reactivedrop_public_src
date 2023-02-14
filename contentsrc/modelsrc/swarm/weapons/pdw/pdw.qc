$modelname "weapons/PDW/PDW.mdl"

$body "default" "pdw_model.smd"

$bodygroup "magazine"
{
	studio "pdw_mags.smd"
	blank
}
$bodygroup "grip"
{
	studio "pdw_grip_in.smd"
	studio "pdw_grip_out.smd"
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/PDW"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0
$attachment "muzzle_flash_l" "muzzle_flash_l" 0 0 0 rotate 0 0 0
$attachment "eject2" "eject2" 0 0 0 rotate 0 0 0

$sequence "BindPose" "pdw_model.smd"

$collisionmodel "pdw_physics.smd"
