$modelname "weapons/Shotgun/Shotgun.mdl"

$body "default" "shotgun_model.smd"

$bodygroup "magazine"
{
	studio "shotgun_barrel_up.smd"
	studio "shotgun_barrel_down.smd"
}
$bodygroup "bolt"
{
	studio "shotgun_lever.smd"
	studio "shotgun_lever_open.smd"
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/Shotgun"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "shotgun_barrel_up.smd"

$collisionmodel "shotgun_physics.smd"
