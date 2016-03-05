#pragma once

const bool FRAMEWORKCOMMUNICATION3_API debug_output( const wchar_t *p_category, const wchar_t *p_format, ... );
const bool FRAMEWORKCOMMUNICATION3_API debug_output( const wchar_t *p_category, const wchar_t *p_format, va_list argptr );

const bool FRAMEWORKCOMMUNICATION3_API debug_cls( void );