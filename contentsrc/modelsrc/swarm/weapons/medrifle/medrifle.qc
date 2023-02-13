$modelname "weapons/medrifle/medrifle.mdl"

$body "default" "medrifle_reference.smd"

$bodygroup "magazine"
{
	studio "medrifle_mag.smd"
	blank
}
$bodygroup "grip"
{
	studio "medrifle_tank.smd"
	blank
}
$bodygroup "sight"
{
	studio "medrifle_sight.smd"
	blank
}
$bodygroup "screen"
{
	studio "medrifle_screen.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/medrifle"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "medrifle_reference.smd"

$collisionmodel "medrifle_physics.smd"
