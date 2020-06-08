#ifndef INCLUDED_VERSION_H
#define INCLUDED_VERSION_H

/*! \brief major version number */
#define XLSXMAGIC_VERSION_MAJOR 0
/*! \brief minor version number */
#define XLSXMAGIC_VERSION_MINOR 1
/*! \brief micro version number */
#define XLSXMAGIC_VERSION_MICRO 0

#define XLSXMAGIC_VERSION_STRINGIZE_(major, minor, micro) #major"."#minor"."#micro
#define XLSXMAGIC_VERSION_STRINGIZE(major, minor, micro) XLSXMAGIC_VERSION_STRINGIZE_(major, minor, micro)
#define XLSXMAGIC_VERSION_STRING XLSXMAGIC_VERSION_STRINGIZE(XLSXMAGIC_VERSION_MAJOR, XLSXMAGIC_VERSION_MINOR, XLSXMAGIC_VERSION_MICRO)


#define XLSXMAGIC_NAME "xlsxmagic"
#define XLSXMAGIC_FULLNAME XLSXMAGIC_NAME " " XLSXMAGIC_VERSION_STRING

#endif
