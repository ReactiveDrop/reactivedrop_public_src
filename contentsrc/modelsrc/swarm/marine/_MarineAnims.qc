//==============================================================================
// Marine Animations
//==============================================================================





//==============================================================================
// the model + basics
//==============================================================================
$modelname swarm/marine/marine_anims.mdl
$addsearchdir dmx/
$addsearchdir dmx/animation/

//==============================================================================
// includes
//==============================================================================
$include "_DefineBones.qci"
$include "_HierarchyChainsWeights.qci"
$include "_BoneMerges.qci"

// idles/walk/run
$include "_BaseAnims.qci"

// stop stand transition anim being inserted between these nodes
$skiptransition "running" "walking"

$include "_WeaponAnims.qci"
$include "_FlinchAnims.qci"

$include "_Attachments.qci"
