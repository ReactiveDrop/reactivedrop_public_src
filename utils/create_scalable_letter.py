from subprocess import check_call
import sys, os

vtex_exe = R"D:\Games\Steam\steamapps\common\Alien Swarm Reactive Drop\bin\vtex.exe"
game_dir = R"D:\Games\Steam\steamapps\common\Alien Swarm Reactive Drop\reactivedrop"
out_dir = "../reactivedrop/materials/vgui/letters"

for letter in sys.argv[1:]:
	codepoint = letter.upper()
	#smooth = ["-blur", "0x10", "-threshold", "50%", "-blur", "0x1"]
	smooth = ["-threshold", "50%", "-blur", "0x1"]
	if len(letter) == 1:
		if ord(letter) < 97 or ord(letter) > 122:
			raise Exception("invalid single letter (must be ASCII and lowercase; otherwise use a codepoint)")
	else:
		if len(letter) != 4 or letter.lower() != letter:
			raise Exception("invalid codepoint (should be 4 lowercase hex digits)")
		codepoint = bytes.fromhex(letter).decode("utf-16be")
		if ord(codepoint) >= 0x2E80:
			# more complicated letters get messed up by rounding; don't do it for CJK
			smooth = ["-threshold", "50%", "-blur", "0x1"]

	with open("letter.utf8", "w", encoding="utf-8") as f:
		f.write(codepoint)

	check_call(["magick", "-background", "#000", "-fill", "#fff", "-font", "Noto-Sans-CJK-SC-Black", "-pointsize", "336", "-gravity", "center", "-size", "512x512", "label:@letter.utf8"] + smooth + ["-resize", "256x256", "xc:#fff", "-channel", "RGB", "-clut", "letter_" + letter + ".tga"])
	check_call(["magick", "-background", "#000", "-fill", "#fff", "-font", "Noto-Sans-CJK-SC-Black", "-pointsize", "336", "-gravity", "center", "-size", "512x512", "label:@letter.utf8"] + smooth + ["(", "-clone", "0", "-blur", "0x15", "-clone", "0,0,0,0,0", "-composite", ")", "-composite", "-resize", "256x256", "xc:#fff", "-channel", "RGB", "-clut", "letter_" + letter + "_glow.tga"])
	check_call([vtex_exe, "-game", game_dir, "-outdir", out_dir, "-nopause", "letter_" + letter + ".tga"])
	check_call([vtex_exe, "-game", game_dir, "-outdir", out_dir, "-nopause", "letter_" + letter + "_glow.tga"])
	os.unlink("letter_" + letter + ".tga")
	os.unlink("letter_" + letter + "_glow.tga")
	os.unlink("letter.utf8")
	with open(out_dir + "/letter_" + letter + ".vmt", "w") as f:
		f.write("""UnlitGeneric
{
$basetexture "vgui\\letters\\letter_%s"
$translucent 1
$distancealpha 1
$softedges 1
$edgesoftnessstart .6
$edgesoftnessend .2
$scaleedgesoftnessbasedonscreenres 1
$vertexcolor 1
$vertexalpha 1
$no_fullbright 1
$ignorez 1
}""" % letter)
	with open(out_dir + "/letter_" + letter + "_glow.vmt", "w") as f:
		f.write("""UnlitGeneric
{
$basetexture "vgui\\letters\\letter_%s_glow"
$translucent 1
$vertexcolor 1
$vertexalpha 1
$ignorez 1
}""" % letter)
