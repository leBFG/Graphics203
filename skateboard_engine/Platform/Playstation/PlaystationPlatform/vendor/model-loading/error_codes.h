/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 6.00.00.38-00.00.00.0.1
* Copyright (C) 2021 Sony Interactive Entertainment Inc.
*/

#ifndef PACK_ERROR_CODES_H
#define PACK_ERROR_CODES_H

/*
 * Error number definition
 */

/**
 * @j メモリ不足 @ej
 * @e Out of memory. @ee
 */
#define SCE_PACK_ERROR_INSUFFICIENT_MEMORY		-2118975487	/* 0x81B30001 */

/**
 * @j 不正な引数が関数に渡された @ej
 * @e Invalid argument passed to a function. @ee
 */
#define SCE_PACK_ERROR_INVALID_PARAM			-2118975486	/* 0x81B30002 */

/**
 * @j ファイル読み込み中にエラーが発生した @ej
 * @e Error while reading the file. @ee
 */
#define SCE_PACK_ERROR_IO						-2118975485	/* 0x81B30003 */

/**
 * @j 予期しないデータが見つかった @ej
 * @e Unexpected data found. @ee
 */
#define SCE_PACK_ERROR_MALFORMED_FILE			-2118975484	/* 0x81B30004 */

/**
 * @j 不明なバージョン @ej
 * @e Unknown version. @ee
 */
#define SCE_PACK_ERROR_VERSION_MISMATCH			-2118975483	/* 0x81B30005 */

/**
 * @j 解析中に予期せずバッファの終わりに到達した @ej
 * @e Unexpectedly reached end of the buffer while parsing. @ee
 */
#define SCE_PACK_ERROR_BUFFER_TOO_SMALL			-2118975482	/* 0x81B30006 */

/**
 * @j シリアライズされたオブジェクトはリクエストされた型には変換できません。 @ej
 * @e The serialized object is not convertible to the requested type. @ee
 */
#define SCE_PACK_ERROR_NOT_CONVERTIBLE			-2118975481	/* 0x81B30007 */

/**
 * @j エクスポータが必須としてマークしたフィールドが見つかりました。このバージョンは解釈できません。 @ej
 * @e The program have found a field that the exporter has marked as mandatory and this version is not able to interpret. @ee
 */
#define SCE_PACK_ERROR_UNKNOWN_MANDATORY_FIELD_FOUND -2118975480	/* 0x81B30008 */


/* Error number definition end */

#endif // PACK_ERROR_CODES_H
