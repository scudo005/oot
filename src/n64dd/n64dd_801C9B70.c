// User-facing error handling
#include "n64dd.h"
#include "z_locale.h"

u8 unk_u8buf2[0x600]; // maybe is to hold the error code texture?
u8 unk_u8buf1[0x2800]; // maybe this is to hold the error body texture?
u8 unk_u8buf0[0x1400];

/**
 * Converts a number in decimal to a hexadecimal number with the same digits, e.g. 1234 -> 0x1234.
 *
 * Will only work on nonnegative numbers.
 *
 * @param decNumber Number in decimal to convert, e.g. 1234
 * @return s32 Hexadecimal number with the same digits as decNumber, e.g. 0x1234
 */
s32 n64ddError_ConvertDecToSameDigitHex(s32 decNumber) {
    s32 currPlaceValue;
    s32 currExponent = 0;
    s32 accumulatedHexDigits = 0;
    s32 remainingDecDigits;

    // Nothing to do if only one digit.
    if (decNumber < 10) {
        return decNumber;
    }

    // Find the place value / exponent of the most significant digit in decNumber.
    for (currPlaceValue = 1; 10 * currPlaceValue <= decNumber; currPlaceValue *= 10) {
        currExponent++;
    }

    remainingDecDigits = decNumber;

    // Transfer the digits to hex one at a time.
    while (currExponent--) {
        // Place the most significant remaining digit at the end of the hex output.
        accumulatedHexDigits |= remainingDecDigits / currPlaceValue;
        accumulatedHexDigits *= 0x10; // Shift left one hex digit.

        remainingDecDigits %= currPlaceValue; // Remove most significant of the remaining digits.

        currPlaceValue /= 10;
    }

    accumulatedHexDigits += remainingDecDigits; // Only one digit left in the remainingDecDigits.
    return accumulatedHexDigits;
}

// n64ddError_GetLanguage
s32 n64ddError_GetLanguage(void) {
    return (gCurrentRegion == 1) ? 0 : 1;
}

// n64ddError_Memset
void n64ddError_Memset(u8* dest, u8 value, u32 count) {
    while (count--) {
        *dest++ = value;
    }
}

// n64ddError_GetErrorHeader
const char* n64ddError_GetErrorHeader(void) {
    return D_801D2ED0[n64ddError_GetLanguage()];
}

// n64ddError_WriteNumberJP
// Writes a 2-digit number to the char buffer provided
// Character indices for numbers in the error code (EUC-JP)
void n64ddError_WriteNumberJP(u8* buf, s32 number) {
    s32 temp_v0 = n64ddError_ConvertDecToSameDigitHex(number);
    u16 character;

    if (number >= 10) {
        character = ((temp_v0 >> 4) + 0xA3B0); // '０'
    } else {
        character = 0xA1A1; // '　'
    }

    func_801C94F8(buf, character);
    buf += 2;
    func_801C94F8(buf, ((temp_v0 & 0xF) + 0xA3B0)); // '０'
}

// n64ddError_WriteNumberEN
// Writes a 2-digit number to the char buffer provided
// Character indices for numbers in the error code (ASCII)
void n64ddError_WriteNumberEN(u8* buf, s32 number) {
    s32 temp_v0 = n64ddError_ConvertDecToSameDigitHex(number);

    if (number >= 10) {
        *buf = (temp_v0 >> 4) + '0';
    } else {
        *buf = ' ';
    }
    buf++;
    *buf = (temp_v0 & 0xF) + '0';
}

void n64ddError_GetLocalizedErrorByLocale(u8* arg0, s32 errorNum) {
    u8* errorHeader = (u8*)n64ddError_GetErrorHeader();

    //! @bug: both of these functions will write to the pointer target, but errorHeader points to a string literal,
    //! which is meant to be const.
    if (gCurrentRegion == 1) {
        n64ddError_WriteNumberJP(&errorHeader[12], errorNum);
    } else {
        n64ddError_WriteNumberEN(&errorHeader[13], errorNum);
    }
    func_801C9A10(arg0, 192, errorHeader);
}

u8* n64ddError_GetPtrToErrorCodeTexture(s32 errorNum) {
    n64dd_clearUnkU8Buf2();

    if (errorNum == 41) {
        return (u8*)gN64DDError41Texs[n64ddError_GetLanguage()]; // get a localized version of the text
                                                                // specifically for error code 41
    }

    // 31,32, 37,38,39,40
    if (((errorNum >= 37) && (errorNum < 41)) || (errorNum == 31) || (errorNum == 32)) {
        return unk_u8buf2;
    } else {
        n64ddError_GetLocalizedErrorByLocale(unk_u8buf2, errorNum);
        return unk_u8buf2;
    }
}

// Clear something
u8* n64dd_clearUnkU8Buf2(void) {
    n64ddError_Memset(unk_u8buf2, 0, 0x600);
    return unk_u8buf2;
}

// Prints the error message body (?)
void n64ddError_PrintErrorMsgBody(u8* arg0, s32 errorNum, s32 lineCount) {
    s32 i;

    for (i = 0; i < lineCount; i++, arg0 += 0xA00) {
        u8* line = (u8*)D_801D2EE0[n64ddError_GetLanguage()][errorNum][i];

        if (1) {}
        func_801C9A10(arg0, 320, line);
    }
}

u8* n64ddError_GetErrorMsgTexture(s32 errorNum) {
    n64dd_clearUnkU8Buf1();
    if (errorNum == 3) {
        return (u8*)gN64DDPleaseReadManualTexs[n64ddError_GetLanguage()]; // ptrs to localized texts?
    }
    n64ddError_PrintErrorMsgBody(unk_u8buf1, errorNum, 4);
    return unk_u8buf1;
}

// Clear something
u8* n64dd_clearUnkU8Buf1(void) {
    n64ddError_Memset(unk_u8buf1, 0, 0x2800);
    return unk_u8buf1;
}

u8* n64ddError_ClearUnkU8Buf0AndPrintErr(s32 errorNum) {
    n64dd_clearUnkU8Buf0();
    n64ddError_PrintErrorMsgBody(unk_u8buf0, errorNum, 2);
    return unk_u8buf0;
}

// Clear something
u8* n64dd_clearUnkU8Buf0(void) {
    n64ddError_Memset(unk_u8buf0, 0, 0x1400);
    return unk_u8buf0;
}
