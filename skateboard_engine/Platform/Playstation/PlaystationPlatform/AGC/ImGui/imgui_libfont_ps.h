/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 6.00.00.38-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#pragma once

#include <mspace.h>
#include "imgui/imgui.h"

/*!
 * @~English
 * @brief ImGui Font library
 * @~Japanese
 * @brief ImGuiフォントライブラリ
 */
namespace ImGuiLibFont
{
    //! @brief Hinting greatly impacts visuals (and glyph sizes).
    //! @details When disabled, FreeType generates blurrier glyphs, more or less matches the stb's output.
    //! The Default hinting mode usually looks good, but may distort glyphs in an unusual way.
    //! The Light hinting mode generates fuzzier glyphs but better matches Microsoft's rasterizer.
    
    //! You can set those flags on a per font basis in ImFontConfig::RasterizerFlags.
    //! Use the 'extra_flags' parameter of BuildFontAtlas() to force a flag on all your fonts.
    enum RasterizerFlags 
    {
        // By default, hinting is enabled and the font's native hinter is preferred over the auto-hinter.
        NoHinting       = 1 << 0,   //! Disable hinting. This generally generates 'blurrier' bitmap glyphs when the glyph are rendered in any of the anti-aliased modes.
        NoAutoHint      = 1 << 1,   //! Disable auto-hinter.
        ForceAutoHint   = 1 << 2,   //! Indicates that the auto-hinter is preferred over the font's native hinter.
        LightHinting    = 1 << 3,   //! A lighter hinting algorithm for gray-level modes. Many generated glyphs are fuzzier but better resemble their original shape. This is achieved by snapping glyphs to the pixel grid only vertically (Y-axis), as is done by Microsoft's ClearType and Adobe's proprietary font renderer. This preserves inter-glyph spacing in horizontal text.
        MonoHinting     = 1 << 4,   //! Strong hinting algorithm that should only be used for monochrome output.
        Bold            = 1 << 5,   //! Styling: Should we artificially embolden the font?
        Oblique         = 1 << 6    //! Styling: Should we slant the font, emulating italic style?
    };

	/*!
	 * @~English
	 * @brief Adds system font to ImGui
	 * @param atlas Font atlas
	 * @param size_pixels Font height(in pixels)
	 * @param font_cfg Font configuration
	 * @param glyph_ranges Character set to be added to font atlas
	 * @return Added system font
	 * @~Japanese
	 * @brief ImGuiにシステムフォントを追加
	 * @param atlas フォントアトラス
	 * @param size_pixels フォントの高さ(ピクセル)
	 * @param font_cfg フォント設定
	 * @param glyph_ranges フォントアトラスに登録する文字セット
	 * @return 追加されたシステムフォント
	 */
    IMGUI_API ImFont *AddSystemFont(ImFontAtlas* atlas, float size_pixels, const ImFontConfig* font_cfg = NULL, const ImWchar* glyph_ranges = NULL);
	/*!
	 * @~English
	 * @brief Clears system font
	 * @param atlas Font atlas
	 * @retval true Success
	 * @retval false Failure
	 * @~Japanese
	 * @brief ImGuiからシステムフォントを削除
	 * @param atlas フォントアトラス
	 * @retval true 成功
	 * @retval false 失敗
	 */
    IMGUI_API bool ClearSystemFont(ImFontAtlas* atlas);

	/*!
	 * @~English
	 * @brief Initializes ImGui font library
	 * @retval >=SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese
	 * @brief ImGuiフォントライブラリ初期化
	 * @retval  SCE_OK 成功。
	 * @retval (<0) エラーコード
	 */
    IMGUI_API int Initialize();
	/*!
	 * @~English
	 * @brief Finalizes ImGui font library
	 * @retval >=SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese
	 * @brief ImGuiフォントライブラリ終了処理
	 * @retval  SCE_OK 成功。
	 * @retval (<0) エラーコード
	 */
    IMGUI_API int Finalize();

	/*!
	 * @~English
	 * @brief Builds font atlas
	 * @param atlas Font atlas
	 * @param mspace Memory allocator
	 * @param extra_flags Logical-OR of RasterizerFlags enum values
	 * @retval true Success
	 * @retval false Failure
	 * @~Japanese
	 * @brief ImGuiフォントアトラス構築
	 * @param atlas フォントアトラス
	 * @param mspace メモリアロケータ
	 * @param extra_flags RasterizerFlags enum値の論理和
	 * @retval true 成功
	 * @retval false 失敗
	 */
	IMGUI_API bool BuildFontAtlas(ImFontAtlas* atlas, SceLibcMspace mspace, unsigned int extra_flags = 0);
	/*!
	 * @~English
	 * @brief Adds specified character set to font atlas
	 * @param atlas Font atlas
	 * @param target Font to be added
	 * @param str Character set to be added
	 * @~Japanese
	 * @brief 指定した文字セットのみをフォントアトラスに追加
	 * @param atlas フォントアトラス
	 * @param target 追加するフォント
	 * @param str 追加する文字セット
	 */
	IMGUI_API void BuildFontAtlasLimitedChar(ImFontAtlas* atlas, ImFont *target, const ImWchar *str);
	/*!
	 * @~English
	 * @brief Removes specified character set from font atlas
	 * @param atlas Font atlas
	 * @param target Font to be removed
	 * @~Japanese
	 * @brief 指定したフォントのみをフォントアトラスから削除
	 * @param atlas フォントアトラス
	 * @param target 削除するフォント
	 */
	IMGUI_API void DestroyFontAtlasLimitedChar(ImFontAtlas* atlas, ImFont *target);
}
