#pragma once

// Use #line macro to change the name of the current file.

/**
 * Options. Define before the first file inclusion if needed.
 */

// #define minidebug_miniprint_FILEptr stdout
// #define minidebug_miniprint_func std::print
// #define minidebug_miniprintf_func fprintf
#define minidebug_colorise_output
#define minidebug_output_prefix minidebug_COLOUR_PREFIX "[mdbg] "


/**
 * End of options.
 */

#define minidebug_expand_str(x) #x
#define minidebug_CSI "\033["
// Use multiple arguments as minidebug_SGR(minidebug_SGR_COLOUR_DEFAULT;minidebug_SGR_COLOUR_F_BRIGHT_YELLOW)
#define minidebug_SGR(minidebug_SGR_colour) minidebug_CSI minidebug_expand_str(minidebug_SGR_colour) "m"

#define minidebug_SGR_COLOUR_DEFAULT 0
#define minidebug_SGR_COLOUR_F_BRIGHT 1
#define minidebug_SGR_COLOUR_F_NOBRIGHT 22
#define minidebug_SGR_COLOUR_F_UNDERLINE 4
#define minidebug_SGR_COLOUR_F_NOUNDERLINE 24
#define minidebug_SGR_COLOUR_NEGATIVE 7
#define minidebug_SGR_COLOUR_NONEGATIVE 27

#define minidebug_SGR_COLOUR_F_BLACK    30
#define minidebug_SGR_COLOUR_F_RED      31
#define minidebug_SGR_COLOUR_F_GREEN    32
#define minidebug_SGR_COLOUR_F_YELLOW   33
#define minidebug_SGR_COLOUR_F_BLUE     34
#define minidebug_SGR_COLOUR_F_MAGENTA  35
#define minidebug_SGR_COLOUR_F_CYAN     36
#define minidebug_SGR_COLOUR_F_WHITE    37
#define minidebug_SGR_COLOUR_F_EXT      38

#define minidebug_SGR_COLOUR_F_DEFAULT  39
#define minidebug_SGR_COLOUR_B_BLACK    40
#define minidebug_SGR_COLOUR_B_RED      41
#define minidebug_SGR_COLOUR_B_GREEN    42
#define minidebug_SGR_COLOUR_B_YELLOW   43
#define minidebug_SGR_COLOUR_B_BLUE     44
#define minidebug_SGR_COLOUR_B_MAGENTA  45
#define minidebug_SGR_COLOUR_B_CYAN     46
#define minidebug_SGR_COLOUR_B_WHITE    47
#define minidebug_SGR_COLOUR_B_EXT      48

#define minidebug_SGR_COLOUR_B_DEFAULT  49
#define minidebug_SGR_COLOUR_F_BRIGHT_BLACK    90
#define minidebug_SGR_COLOUR_F_BRIGHT_RED      91
#define minidebug_SGR_COLOUR_F_BRIGHT_GREEN    92
#define minidebug_SGR_COLOUR_F_BRIGHT_YELLOW   93
#define minidebug_SGR_COLOUR_F_BRIGHT_BLUE     94
#define minidebug_SGR_COLOUR_F_BRIGHT_MAGENTA  95
#define minidebug_SGR_COLOUR_F_BRIGHT_CYAN     96
#define minidebug_SGR_COLOUR_F_BRIGHT_WHITE    97
#define minidebug_SGR_COLOUR_B_BRIGHT_BLACK    100
#define minidebug_SGR_COLOUR_B_BRIGHT_RED      101
#define minidebug_SGR_COLOUR_B_BRIGHT_GREEN    102
#define minidebug_SGR_COLOUR_B_BRIGHT_YELLOW   103
#define minidebug_SGR_COLOUR_B_BRIGHT_BLUE     104
#define minidebug_SGR_COLOUR_B_BRIGHT_MAGENTA  105
#define minidebug_SGR_COLOUR_B_BRIGHT_CYAN     106
#define minidebug_SGR_COLOUR_B_BRIGHT_WHITE    107


#ifndef minidebug_miniprint_FILEptr
    #define minidebug_miniprint_FILEptr stdout
#endif
#ifndef minidebug_miniprint_func
    #define minidebug_miniprint_func std::print
#endif
#ifndef minidebug_miniprintf_func
    #define minidebug_miniprintf_func fprintf
#endif

#ifdef minidebug_colorise_output
    #define minidebug_COLOUR_DEFAULT    minidebug_SGR(minidebug_SGR_COLOUR_DEFAULT)
    #define minidebug_COLOUR_MSG        minidebug_SGR(minidebug_SGR_COLOUR_DEFAULT;minidebug_SGR_COLOUR_F_BRIGHT_YELLOW)
    #define minidebug_COLOUR_FUNC       minidebug_SGR(minidebug_SGR_COLOUR_F_BRIGHT_MAGENTA)
    #define minidebug_COLOUR_FILE       minidebug_SGR(minidebug_SGR_COLOUR_F_UNDERLINE;minidebug_SGR_COLOUR_F_DEFAULT)
    #define minidebug_COLOUR_LINE       minidebug_SGR(minidebug_SGR_COLOUR_F_NOUNDERLINE)
    #define minidebug_COLOUR_PREFIX     minidebug_SGR(minidebug_SGR_COLOUR_F_BRIGHT_YELLOW)
#else
    #define minidebug_COLOUR_DEFAULT
    #define minidebug_COLOUR_MSG
    #define minidebug_COLOUR_FUNC
    #define minidebug_COLOUR_FILE
    #define minidebug_COLOUR_LINE
    #define minidebug_COLOUR_PREFIX
#endif

#define minidebug_loc_line_   minidebug_COLOUR_FILE "{}" minidebug_COLOUR_LINE ":{}, " minidebug_COLOUR_FUNC "{}()"
#define minidebug_loc_line_f_ minidebug_COLOUR_FILE "%s" minidebug_COLOUR_LINE ":%d, " minidebug_COLOUR_FUNC "%s()"
#define minidebug_loc_line_ex_ minidebug_loc_line_ ":" minidebug_COLOUR_MSG " "
#define minidebug_loc_line_f_ex_ minidebug_loc_line_f_ ":" minidebug_COLOUR_MSG " "
// wtf is __BASE_FILE__

/**
 * @brief Prints the location of itself in the "file:line, func()" format.
 */
#define miniprintloc() minidebug_miniprint_func(minidebug_miniprint_FILEptr, minidebug_output_prefix minidebug_loc_line_ minidebug_COLOUR_DEFAULT, __FILE__, __LINE__, __func__)
#define miniprintlocln() minidebug_miniprint_func(minidebug_miniprint_FILEptr, minidebug_output_prefix minidebug_loc_line_ minidebug_COLOUR_DEFAULT "\n", __FILE__, __LINE__, __func__)
/**
 * @brief Prints the location of itself in the "file:line, func()" format.
 */
#define miniprintfloc() minidebug_miniprintf_func(minidebug_miniprint_FILEptr, minidebug_output_prefix  minidebug_loc_line_f_ minidebug_COLOUR_DEFAULT, __FILE__, __LINE__, __func__)
#define miniprintflocln() minidebug_miniprintf_func(minidebug_miniprint_FILEptr, minidebug_output_prefix minidebug_loc_line_f_ minidebug_COLOUR_DEFAULT "\n", __FILE__, __LINE__, __func__)

// https://stackoverflow.com/a/48045656
#if __cplusplus <= 201703 && defined __GNUC__ && !defined __clang__ && !defined __EDG__ // These compilers pretend to be GCC
    #define VA_OPT_SUPPORTED false
#else
    #define PP_THIRD_ARG(a,b,c,...) c
    #define VA_OPT_SUPPORTED_I(...) PP_THIRD_ARG(__VA_OPT__(,),true,false,)
    #define VA_OPT_SUPPORTED VA_OPT_SUPPORTED_I(?)
#endif

#if VA_OPT_SUPPORTED
    #define miniprint(format_, ...) minidebug_miniprint_func(minidebug_miniprint_FILEptr, \
        minidebug_output_prefix minidebug_loc_line_ex_ format_ minidebug_COLOUR_DEFAULT, \
        __FILE__, __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__)
    #define miniprintf(format_, ...) minidebug_miniprintf_func(minidebug_miniprint_FILEptr, \
        minidebug_output_prefix minidebug_loc_line_f_ex_ format_ minidebug_COLOUR_DEFAULT, \
        __FILE__, __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__)
#elif defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL
// Traditional preprocessor removes the variadic args comma automatically.
    #define miniprint(format_, ...) minidebug_miniprint_func(minidebug_miniprint_FILEptr, \
        minidebug_output_prefix minidebug_loc_line_ex_ format_ minidebug_COLOUR_DEFAULT, \
        __FILE__, __LINE__, __func__, __VA_ARGS__)
    #define miniprintf(format_, ...) minidebug_miniprintf_func(minidebug_miniprint_FILEptr, \
        minidebug_output_prefix minidebug_loc_line_f_ex_ format_ minidebug_COLOUR_DEFAULT, \
        __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
// No VA_OPT but new preprocessor, which uses common ## prefix to remove the comma.
    #define miniprint(format_, ...) minidebug_miniprint_func(minidebug_miniprint_FILEptr, \
        minidebug_output_prefix minidebug_loc_line_ex_ format_ minidebug_COLOUR_DEFAULT, \
        __FILE__, __LINE__, __func__ , ## __VA_ARGS__)
    #define miniprintf(format_, ...) minidebug_miniprintf_func(minidebug_miniprint_FILEptr, \
        minidebug_output_prefix minidebug_loc_line_f_ex_ format_ minidebug_COLOUR_DEFAULT, \
        __FILE__, __LINE__, __func__ , ## __VA_ARGS__)
#endif


/**
 * @brief miniprintval("{} cm", my_val); --> "my_val = 72 cm\n"
 */
#define miniprintval(format_, ...) minidebug_miniprint_func(minidebug_miniprint_FILEptr, #__VA_ARGS__ " = " format_ "\n", __VA_ARGS__ )
/**
 * @brief miniprintfval("%d cm", my_val); --> "my_val = 72 cm\n"
 */
#define miniprintfval(format_, ...) minidebug_miniprintf_func(minidebug_miniprint_FILEptr, #__VA_ARGS__ " = " format_ "\n", __VA_ARGS__)
/**
 * @brief miniprintval("{} cm", my_val); --> "my_val = 72 cm\n"
 */
#define miniprintvalloc(format_, ...) miniprint(#__VA_ARGS__ " = " format_ "\n", __VA_ARGS__ )
/**
 * @brief miniprintfval("%d cm", my_val); --> "my_val = 72 cm\n"
 */
#define miniprintfvalloc(format_, ...) miniprintf(#__VA_ARGS__ " = " format_ "\n", __VA_ARGS__)


/* EXAMPLES
    float value = 7.2f;

    miniprintloc();
        std::print((__acrt_iob_func(1)), "{}:{}, {}(): ", "E:\\dev\\libs\\minidebug\\include\\minidebug.h", 57, __func__);

    miniprint("test!");
        miniprintloc(); std::print((__acrt_iob_func(1)), "test!");
    
    miniprintval("{}", value);
        miniprintloc(); std::print((__acrt_iob_func(1)), "value" " = " "{}" "\n", value __VA_OPT__(,));;



    miniprintfloc();
        fprintf((__acrt_iob_func(1)), "%s:%d, %s(): ", "E:\\dev\\libs\\minidebug\\include\\minidebug.h", 68, __func__);

    miniprintf("test!");
        miniprintfloc(); fprintf((__acrt_iob_func(1)), "test!");
    
    miniprintfval("{%i; %i}", v.x, v.y);
        fprintf((__acrt_iob_func(1)), "v.x, v.y" " = " "{%i; %i}" "\n", v.x, v.y);;
*/

// #include <iostream>

// int main(){
//     miniprintf("Amazing test");
//     return 0;
// }
