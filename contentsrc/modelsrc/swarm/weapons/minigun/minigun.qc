$modelname "weapons/MiniGun/MiniGun.mdl"

$body "default" "minigun_model.smd"

$bodygroup "magazine"
{
	studio "minigun_barrel.smd"
	blank
}
$bodygroup "carryhandle"
{
	studio "minigun_carryhandle.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/AutoGun"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "minigun_model.smd"

$sequence "fire" {
	"anims/fire.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	fps 48
	loop
}

$sequence "idle" {
	"anims/idle.smd"
	activity "ACT_VM_IDLE" 1
	fps 30
}

$collisionmodel "minigun_physics.smd"
