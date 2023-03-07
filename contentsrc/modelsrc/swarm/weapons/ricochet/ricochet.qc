$modelname "weapons/ricochet/ricochet.mdl"

$body "default" "ric_model3.dmx"

$bodygroup "magazine"
{
	studio "ric_mag.dmx"
	blank
}
$bodygroup "grip"
{
	studio "ric_grip.dmx"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/ricochet"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "ric_model3.dmx"

$collisionmodel "ric_physics.smd"
