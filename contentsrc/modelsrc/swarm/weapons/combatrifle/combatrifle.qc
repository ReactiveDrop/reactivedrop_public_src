$modelname "weapons/CombatRifle/CombatRifle.mdl"

$body "default" "combatrifle_reference.smd"

$bodygroup "magazine"
{
	studio "combatrifle_mag.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/CombatRifle"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "combatrifle_reference.smd"

$collisionmodel "combatrifle_physics.smd"
