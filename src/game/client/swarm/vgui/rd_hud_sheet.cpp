#include "cbase.h"
#include "rd_hud_sheet.h"
#include "bitmap/psheet.h"
#include "vgui_controls/Controls.h"
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CRD_HUD_Sheets g_RD_HUD_Sheets;

void CRD_HUD_Sheets::VidInit()
{
	for ( int i = 0; i < m_HudSheets.Count(); i++ )
	{
		if ( *( m_HudSheets[i].m_pSheetID ) == -1 )
		{
			*( m_HudSheets[i].m_pSheetID ) = vgui::surface()->CreateNewTextureID();
			vgui::surface()->DrawSetTextureFile( *( m_HudSheets[i].m_pSheetID ), m_HudSheets[i].m_pszTextureFile, true, false );

			// pull out UV coords for this sheet
			ITexture *pTexture = materials->FindTexture( m_HudSheets[i].m_pszTextureFile, TEXTURE_GROUP_VGUI );
			if ( pTexture )
			{
				m_HudSheets[i].m_pSize->x = pTexture->GetActualWidth();
				m_HudSheets[i].m_pSize->y = pTexture->GetActualHeight();
				size_t numBytes;
				void const *pSheetData = pTexture->GetResourceData( VTF_RSRC_SHEET, &numBytes );
				if ( pSheetData )
				{
					CUtlBuffer bufLoad( pSheetData, numBytes, CUtlBuffer::READ_ONLY );
					CSheet *pSheet = new CSheet( bufLoad );
					for ( int k = 0; k < pSheet->m_SheetInfo.Count(); k++ )
					{
						if ( k >= m_HudSheets[i].m_nNumSubTextures )
						{
							break;
						}
						SequenceSampleTextureCoords_t &Coords = pSheet->m_SheetInfo[k].m_pSamples->m_TextureCoordData[0];
						m_HudSheets[i].m_pTextureData[k].u = Coords.m_fLeft_U0;
						m_HudSheets[i].m_pTextureData[k].v = Coords.m_fTop_V0;
						m_HudSheets[i].m_pTextureData[k].s = Coords.m_fRight_U0;
						m_HudSheets[i].m_pTextureData[k].t = Coords.m_fBottom_V0;
					}
				}
				else
				{
					Warning( "Error finding VTF_RSRC_SHEET for %s\n", m_HudSheets[i].m_pszTextureFile );
				}
			}
			else
			{
				Warning( "Error finding %s\n", m_HudSheets[i].m_pszTextureFile );
			}
		}
	}
}
