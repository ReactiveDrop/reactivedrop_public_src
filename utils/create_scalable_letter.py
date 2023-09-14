from subprocess import check_call
from PIL import ImageFont
import sys, os

vtex_exe = R"D:\Games\Steam\steamapps\common\Alien Swarm Reactive Drop\bin\vtex.exe"
game_dir = R"D:\Games\Steam\steamapps\common\Alien Swarm Reactive Drop\reactivedrop"
out_dir = "../reactivedrop/materials/vgui/letters"
out_inc = "../src/game/client/swarm/vgui/asw_scalable_text.inc"
font_latin = "fonts/Play-Bold.ttf"
font_cyrillic = "fonts/Play-Bold.ttf"
#font_han = "fonts/NotoSansCJK-Bold.ttc"
font_han = "fonts/ZCOOLQingKeHuangYou-Regular.ttf"

font_latin_ttf = ImageFont.truetype(font_latin, 336)
font_cyrillic_ttf = ImageFont.truetype(font_cyrillic, 336)
font_han_ttf = ImageFont.truetype(font_han, 336)

with open(out_inc, "w", encoding="utf-16") as inc:
	for letter in sys.argv[1:]:
		font = font_latin
		font_ttf = font_latin_ttf
		category = "L"
		codepoint = letter.upper()
		if len(letter) == 1:
			if ord(letter) < 97 or ord(letter) > 122:
				raise Exception("invalid single letter (must be ASCII and lowercase; otherwise use a codepoint)")
		else:
			if len(letter) != 4 or letter.lower() != letter:
				raise Exception("invalid codepoint (should be 4 lowercase hex digits)")
			codepoint = bytes.fromhex(letter).decode("utf-16be")
			if ord(codepoint) >= 0x0400 and ord(codepoint) < 0x0500:
				font = font_cyrillic
				font_ttf = font_cyrillic_ttf
				category = "C"
			elif ord(codepoint) >= 0x2E80:
				font= font_han
				font_ttf = font_han_ttf
				category = "H"

		with open("letter.utf8", "w", encoding="utf-8") as f:
			f.write(codepoint)

		check_call(["magick", "-background", "#000", "-fill", "#fff", "-font", font, "-pointsize", "336", "-gravity", "center", "-size", "512x512", "label:@letter.utf8", "-threshold", "50%", "-blur", "0x1", "-resize", "256x256", "xc:#fff", "-channel", "RGB", "-clut", "letter_" + letter + ".tga"])
		check_call(["magick", "-background", "#000", "-fill", "#fff", "-font", font, "-pointsize", "336", "-gravity", "center", "-size", "512x512", "label:@letter.utf8", "-threshold", "50%", "-blur", "0x1", "(", "-clone", "0", "-blur", "0x15", "-clone", "0,0,0,0,0", "-composite", ")", "-composite", "-resize", "256x256", "xc:#fff", "-channel", "RGB", "-clut", "letter_" + letter + "_glow.tga"])
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
		inc.write("DECLARE_SCALABLE_LETTER( \"" + letter + "\", L'" + codepoint + "', " + str(font_ttf.getlength(codepoint)) + "f, '" + category + "' )\n")
