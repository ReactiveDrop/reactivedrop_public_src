$modelname "weapons/CryoCannon/CryoCannon.mdl"

$body "default" "cyro_model.smd"

$bodygroup "magazine"
{
	studio "cryo_battery.smd"
	blank
}
$bodygroup "grip"
{
	studio "cryo_grip.smd"
	blank
}
$bodygroup "carryhandle"
{
	studio "cryo_rail.smd"
	blank
}
$bodygroup "screen"
{
	studio "cryo_tablet.smd"
	blank
}
$bodygroup "maw"
{
	studio "cryo_barrel.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/CryoCannon"

$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0
$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0

$sequence "BindPose" "cyro_model.smd"

$collisionmodel "cryocannon_physics.smd"
