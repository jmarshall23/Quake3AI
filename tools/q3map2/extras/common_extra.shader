// ydnar: antiportal works like hint, but supresses portals
// add this to your common.shader file
textures/common/antiportal
{
	qer_nocarve
	qer_trans 0.30
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm structural
	surfaceparm trans
	surfaceparm noimpact
	surfaceparm antiportal
}


// ydnar: skip works like quake 2 hint: it doesn't generate bsp splits
// use on sides of hint brushes where you don't want bsp splits or portals
// add this to your common.shader file
textures/common/skip
{
	qer_nocarve
	qer_trans 0.30
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm structural
	surfaceparm trans
	surfaceparm noimpact
	surfaceparm skip
}
