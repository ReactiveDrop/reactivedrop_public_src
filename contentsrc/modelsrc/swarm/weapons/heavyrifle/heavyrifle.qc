$modelname "weapons/heavyrifle/heavyrifle.mdl"

$body "default" "heavyrifle_reference.smd"

$bodygroup "magazine"
{
	studio "heavyrifle_mag.smd"
	blank
}
$bodygroup "laser"
{
	studio "heavyrifle_laser.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/heavyrifle"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "heavyrifle_reference.smd"
$sequence "pull_bolt" {
	"anims/pull_bolt.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
}

$collisionmodel "heavyrifle_physics.smd"
