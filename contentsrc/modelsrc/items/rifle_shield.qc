
$modelname "items/shield_bubble/rifle_shield.mdl"

$scale 0.5
$body default "rifle_shield.smd"
$surfaceprop "no_decal"
$contents "solid"
$cdmaterials "models/items/shield_bubble"

$sequence "BindPose" "rifle_shield.smd"

$collisionmodel "rifle_shield_physics.smd"
{
	$concave
	$maxconvexpieces 13
}
