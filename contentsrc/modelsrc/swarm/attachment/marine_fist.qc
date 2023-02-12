$modelname swarm/marine/attachments/fist.mdl
$cdmaterials models/player/male
$body equip "marine_fist.smd"

$texturegroup skinfamilies
{
	{ "medic_body" "medic_helmet" }
	{ "officer_body" "officer_helmet" }
	{ "medic_body" "medic_helmet" }
	{ "special_weapons_body" "special_weapons_helmet" }
	{ "tech_body" "tech_helmet" }
}

$include "../marine/_DefineBones.qci"
$include "../marine/_HierarchyChainsWeights.qci"

$sequence BindPose "marine_fist.smd"
$contents solid
$surfaceprop armorflesh
$opaque
