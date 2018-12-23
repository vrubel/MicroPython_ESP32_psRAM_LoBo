/*
 * Module for the Neopixel from the project "indicator-fun"
 *
 * Copyright (c) 2018 Alex Vrubel
 *
 *
 * This file is part of the MicroPython ESP32 project,
 * https://github.com/loboris/MicroPython_ESP32_psRAM_LoBo
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 LoBo (https://github.com/loboris)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdint.h>
#include <stdio.h>
#include "py/runtime.h"


// Опережающее объявление типа (т.к. функции ниже ссылаются на тип, но сам тип
// фактически будет определен в самом конце файла)
extern const mp_obj_type_t ifun_neopixel_type;


//==============================================================================
// Структура данных класса (экземпляр класса - экземпляр данной структуры)
// В данном случае она пустая. Нет определенных членов класса
typedef struct _ifun_neopixel_obj_t
{
	mp_obj_base_t base;

} ifun_neopixel_obj_t;


//==============================================================================
// Функция печати экземпляра класса
STATIC void ifun_neopixel_print(
		const mp_print_t *print,
		mp_obj_t self_in,
		mp_print_kind_t kind)
{
	mp_printf(print, "Privet\n");
}


//==============================================================================
// Конструктор экземпляра класса
STATIC mp_obj_t ifun_neopixel_make_new(
		const mp_obj_type_t *type,
		size_t n_args,
		size_t n_kw,
		const mp_obj_t *all_args)
{
	const mp_arg_t ifun_neopixel_init_allowed_args[] = {
	};
	mp_arg_val_t args[MP_ARRAY_SIZE(ifun_neopixel_init_allowed_args)];
	mp_arg_parse_all_kw_array(
			n_args,
			n_kw,
			all_args,
			MP_ARRAY_SIZE(ifun_neopixel_init_allowed_args),
			ifun_neopixel_init_allowed_args,
			args);

	// Setup the object
	ifun_neopixel_obj_t *self = m_new_obj(ifun_neopixel_obj_t);
	self->base.type = &ifun_neopixel_type;

	return MP_OBJ_FROM_PTR(self);
}


//==============================================================================
// Функция класса. Возвращает значение параметра умноженное на два
STATIC mp_obj_t add(mp_obj_t self_in, mp_obj_t num)
{
	int32_t res = (int32_t)mp_obj_get_int(num)*2;
    return mp_obj_new_int(res);
}
MP_DEFINE_CONST_FUN_OBJ_2(add_obj, add);


//==============================================================================
// Определение таблицы состава класса
// Указываются члены класса
STATIC const mp_rom_map_elem_t ifun_neopixel_locals_dict_table[] =
{
	{ MP_ROM_QSTR(MP_QSTR_add), (mp_obj_t)&add_obj }
};
STATIC MP_DEFINE_CONST_DICT(ifun_neopixel_locals_dict,
	ifun_neopixel_locals_dict_table);


//==============================================================================
// Определение типа класса
const mp_obj_type_t ifun_neopixel_type =
{
	{&mp_type_type},
	.name = MP_QSTR_IFunNeopixel,
	.print = ifun_neopixel_print,
	.make_new = ifun_neopixel_make_new,
	.locals_dict = (mp_obj_dict_t*)&ifun_neopixel_locals_dict,
};


//==============================================================================
// Определение таблицы состава модуля
STATIC const mp_rom_map_elem_t ifun_module_globals_table[] =
{
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_ifun) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_IFunNeopixel), MP_ROM_PTR(&ifun_neopixel_type) }
};
STATIC MP_DEFINE_CONST_DICT(ifun_module_globals, ifun_module_globals_table);

// Определение модуля
const mp_obj_module_t mp_module_ifun =
{
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t*)&ifun_module_globals,
};
