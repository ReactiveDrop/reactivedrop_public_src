$modelname "weapons/RailGun/Railgun.mdl"

$body "default" "railgun_model.smd"

$bodygroup "magazine"
{
	studio "railgun_mag.smd"
	blank
}
$bodygroup "scope"
{
	studio "railgun_scope.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/Railgun"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "railgun_model.smd"

$collisionmodel "railgun_physics.smd"

