$modelname "weapons/Flamethrower/Flamethrower.mdl"

$body "default" "flamethrower_model.smd"

$bodygroup "magazine"
{
	studio "flamethrower_tanks.smd"
	blank
}
$bodygroup "grip"
{
	studio "flamethrower_grip.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/Flamer"

$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0
$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "flame" "ValveBiped.Bip01_R_Hand" 34.6 -4.26 4.5 rotate 90 -45 0

$sequence "BindPose" "flamethrower_model.smd"

$collisionmodel "flamethrower_physics.smd"
