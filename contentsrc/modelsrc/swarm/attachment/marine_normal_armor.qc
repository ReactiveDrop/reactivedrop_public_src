$modelname swarm/marine/attachments/normal_armor.mdl
$cdmaterials models/player/male
$body equip "marine_normal_armor.smd"

$texturegroup skinfamilies
{
	{ "medic_body" }
	{ "officer_body" }
	{ "medic_body" }
	{ "special_weapons_body" }
	{ "tech_body" }
}

$include "../marine/_DefineBones.qci"
$include "../marine/_HierarchyChainsWeights.qci"

$sequence BindPose "marine_normal_armor.smd"
$contents solid
$surfaceprop armorflesh
$opaque
