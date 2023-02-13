$modelname "weapons/AutoGun/Autogun.mdl"

$body "default" "autogun_model.smd"
$bodygroup "magazine"
{
	studio "autogun_barrel.smd"
	blank
}
$bodygroup "carryhandle"
{
	studio "autogun_carryhandle.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/AutoGun"

$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0
$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0

$sequence "BindPose" "autogun_model.smd"

$sequence "fire" {
	"anim/fire.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	fps 48
	loop
}

$sequence "idle" {
	"anim/idle.smd"
	activity "ACT_VM_IDLE" 1
	fps 30
	loop
}

$collisionmodel "autogun_physics.smd"
