$modelname "weapons/ricochet/ricochet.mdl"

$body "default" "ric_model3.smd"

$bodygroup "magazine"
{
	studio "ric_mag.smd"
	blank
}
$bodygroup "grip"
{
	studio "ric_grip.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$illumposition 0.842 0.054 4.415

$cdmaterials "models/weapons/ricochet"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "ric_model3.smd"

$collisionmodel "ric_physics.smd"
