$modelname "weapons/AssaultRifle/AssaultRifle.mdl"

$body "default" "assaultrifle_model.smd"

$bodygroup "magazine"
{
	studio "assaultrifle_magazine.smd"
	blank
}
$bodygroup "legs"
{
	studio "assaultrifle_legs_c.smd"
	studio "assaultrifle_legs_o.smd"
}
$bodygroup "grenadelauncher"
{
	studio "assaultrifle_underslung.smd"
	blank
}

$surfaceprop "weapon"
$contents "solid"
$opaque

$cdmaterials "models/weapons/AssaultRifle"

$attachment "muzzle" "muzzle" 0 0 0 rotate 0 0 0
$attachment "eject1" "eject1" 0 0 0 rotate 0 0 0

$sequence "BindPose" "assaultrifle_model.smd"

$collisionmodel "assaultrifle_physics.smd"
