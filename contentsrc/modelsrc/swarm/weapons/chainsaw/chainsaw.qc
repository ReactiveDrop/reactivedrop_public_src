$modelname "weapons/Chainsaw/chainsaw.mdl"

$body "default" "chainsaw_model.smd"

$bodygroup "grip"
{
	studio "chainsaw_grip.smd"
	studio "chainsaw_grip_combat.smd"
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/Chainsaw"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject" "eject" 0 0 0 rotate 0 0 0

$sequence "BindPose" "chainsaw_model.smd"

$sequence "fire" {
	"anim/fire.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	fps 42
	loop
}

$animation gesture_vertical_blade_base "anim/gesture_vertical_blade.smd" frames 0 0
$animation gesture_vertical_blade_a "anim/gesture_vertical_blade.smd" frames 1 1 subtract gesture_vertical_blade_base 0
$sequence gesture_vertical_blade { gesture_vertical_blade_a loop delta }

$collisionmodel "chainsaw_physics.smd"
