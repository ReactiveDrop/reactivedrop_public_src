$modelname "weapons/Prototype/PrototypeRifle.mdl"

$body "default" "prototyperifle_model.smd"

$bodygroup "magazine"
{
	studio "prototyperifle_mag.smd"
	blank
}
$bodygroup "scope"
{
	studio "prototyperifle_scope.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models\weapons\Prototype"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "prototyperifle_model.smd"
$sequence "pull_bolt" {
	"anims/pull_bolt.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
}

$collisionmodel "prototyperifle_physics.smd"
