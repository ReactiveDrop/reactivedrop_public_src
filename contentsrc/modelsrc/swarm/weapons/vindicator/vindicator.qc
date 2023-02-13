$modelname "weapons/Vindicator/Vindicator.mdl"

$body "default" "vindicator_model2.smd"

$bodygroup "magazine"
{
	studio "vindicator_mag.smd"
	blank
}
$bodygroup "underslung"
{
	studio "vindicator_pump_underslung.smd"
	studio "vindicator_pump.smd"
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/Vindicator"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "vindicator_model2.smd"
$sequence "pull_bolt" {
	"anims/pull_bolt.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
}

$collisionmodel "vindicator_physics.smd"
