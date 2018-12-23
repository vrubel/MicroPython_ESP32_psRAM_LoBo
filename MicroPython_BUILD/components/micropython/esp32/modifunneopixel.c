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
#include <math.h>
#include "driver/rmt.h"
#include "py/runtime.h"


#define RMT_TX_CHANNEL  RMT_CHANNEL_0   // Номер используемого драйвером канала RMT
#define RMT_TX_GPIO     18              // Пин управляющий линейкой светодиодов
#define COUNT_LEDS      20              // Кол-во светодиодов в линейке
#define CLK_DEV         4               // Делитель тактовой, для получения длительности тика, используемого RMT

// Генерируемые длительности, заданные в кол-ве тактовых периодов.
// Тактовый период получается путем деления тактовой частоты (80Mhz) на делитель.
// Делитель выберем равным 4, что дает 50 наносекунд. То есть мы можем определять генерируемые длительности сигналов с точностью 50 наносекунд.
// Информацию о требуемых длительностях импульсов, управляющих светодиодами, возьмем из документации.
// https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf
// 80Mhz / CLK_DEV = 50 наносекунд
#define T0H 8   // 50 * 8 = 400 = 0.4us
#define T0L 17  // 0.85us
#define T1H 16  // 0.8us
#define T1L 9   // 0.45us

// Определим тайминги
static const rmt_item32_t bit0 = {{{ T0H, 1, T0L, 0 }}}; //Logical 0
static const rmt_item32_t bit1 = {{{ T1H, 1, T1L, 0 }}}; //Logical 1


// Фукция транстлятор, для преобразования байта в структуры RMT.
// Вызывается из глубин SDK, когда мы просим SDK передать массив байтов.
// Convert uint8_t type of data to rmt format data.
static void IRAM_ATTR u8_to_rmt(const void* src, rmt_item32_t* dest, size_t src_size,
                         size_t wanted_num, size_t* translated_size, size_t* item_num)
{
    if(src == NULL || dest == NULL) {
        *translated_size = 0;
        *item_num = 0;
        return;
    }

    size_t size = 0;
    size_t num = 0;
    uint8_t *psrc = (uint8_t *)src;
    rmt_item32_t* pdest = dest;
    while (size < src_size && num < wanted_num) {
        for(int i = 0; i < 8; i++) {
            if(*psrc & (0x1 << i)) {
                pdest->val =  bit1.val;
            } else {
                pdest->val =  bit0.val;
            }
            num++;
            pdest++;
        }
        size++;
        psrc++;
    }
    *translated_size = size;
    *item_num = num;
}


/*
 * Initialize the RMT Tx channel
 */
static void rmt_tx_init()
{
    rmt_config_t config;
    config.rmt_mode = RMT_MODE_TX;
    config.channel = RMT_TX_CHANNEL;
    config.gpio_num = RMT_TX_GPIO;
    config.mem_block_num = 1;               // Из всей доступной контроллеру RMT памяти, используется только одна ячейка,
                                            // т.к. данные не подгатавливаются в данной памяти заранее, а каждое выводимое значение генерируется на лету и тут же выводится.
    config.tx_config.loop_en = 0;           // Никаких повторов
    config.tx_config.carrier_en = 0;        // Никакой генерации несущей
    config.tx_config.idle_output_en = 1;    // Периоды простоя между генерируемыми импульсами заполнять значением
    config.tx_config.idle_level = 0;        // Уровень, который используется для вывода в период простоя
    config.clk_div = CLK_DEV;               // Делитель тактовой (80Мгц), обеспечить длительность тика в 50 наносекунд.
                                            // (Длительности генерируемых периодов задаются указаниев кол-ва тиков.
                                            // То есть они будут кратны 50 наносекундам)

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
    ESP_ERROR_CHECK(rmt_translator_init(config.channel, u8_to_rmt));
}


// Опережающее объявление типа (т.к. функции ниже ссылаются на тип, но сам тип
// фактически будет определен в самом конце файла)
extern const mp_obj_type_t ifun_neopixel_type;


//==============================================================================
// Структура данных класса (экземпляр класса - экземпляр данной структуры)
// В данном случае она пустая. Нет определенных членов класса
typedef struct _ifun_neopixel_obj_t {
    mp_obj_base_t base;
    uint8_t rgb_data[COUNT_LEDS * 3];   // Здесь 3 - кол-во байт в последовательности,
                                        // для управления одним светодиодом (для RGB это 3, для RGBW это 4)
    mp_print_t *print;
} ifun_neopixel_obj_t;



//==============================================================================
// Функция печати экземпляра класса
STATIC void ifun_neopixel_print(
        const mp_print_t *print,
        mp_obj_t self_in,
        mp_print_kind_t kind)
{
    mp_printf(print, "Neopixel(Pin=%d, Pixels: %d\n", RMT_TX_GPIO, COUNT_LEDS);
    mp_printf(print, "         Timings (ns): T1H=%d, T1L=%d, T0H=%d, T0L=%d\n)", T1H*50, T1L*50, T0H*50, T0L*50);

    ifun_neopixel_obj_t *self = self_in;
    self->print = print;
}


//==============================================================================
// Конструктор экземпляра класса
STATIC mp_obj_t ifun_neopixel_make_new(
        const mp_obj_type_t *type,
        size_t n_args,
        size_t n_kw,
        const mp_obj_t *all_args)
{
    enum { ARG_pin, ARG_pixels, ARG_type };
    //-----------------------------------------------------
    const mp_arg_t ifun_neopixel_init_allowed_args[] = {
            { MP_QSTR_pin, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} }
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(ifun_neopixel_init_allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(ifun_neopixel_init_allowed_args), ifun_neopixel_init_allowed_args, args);

    // Setup the neopixels object
    ifun_neopixel_obj_t *self = m_new_obj(ifun_neopixel_obj_t);
    self->base.type = &ifun_neopixel_type;

    rmt_tx_init();

    return MP_OBJ_FROM_PTR(self);
}


//==============================================================================
// Деинициализатор экземпляра класса. Особождает ресурсы, занятые экземляром.
STATIC mp_obj_t ifun_neopixel_deinit(mp_obj_t self_in)
{
    rmt_driver_uninstall(RMT_TX_CHANNEL);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(ifun_neopixel_deinit_obj, ifun_neopixel_deinit);


// Функция меняет на обратную последовательность бит в байте.
// Старшие становятся младшими, а младшие старшими
STATIC uint8_t reverse_bits(uint8_t original)
{
    uint8_t reversed = 0;
    for(uint8_t i=0;i<8;i++)
        reversed |= ((original>>i) & 1)<<(7-i);
    return reversed;
}



// Функция установки цвета светодиода (или нескольких последовательно)
// Параметры:
// pos      - номер светодиода в цепочке
// color    - устанавливай цвет в виде целого числа, в котором каждые 8 бит - одна цветовая составляющая (0xRRGGBB)
// num      - кол-во светодиодов (начиная с позиции pos) которым будет установлен цвет
// update   - сгенерировать сигналы упрвления цепочной светодиодов
STATIC mp_obj_t _ifun_neopixel_set(ifun_neopixel_obj_t *self, uint8_t pos, uint8_t R, uint8_t G, uint8_t B, uint8_t count, bool update)
{
    if(pos > COUNT_LEDS) pos = COUNT_LEDS;
    if(pos < 1) pos = 1;
    if(count - pos > COUNT_LEDS - 1)
    {
        count = COUNT_LEDS - pos + 1;
    }

    for(uint8_t i=pos-1; i<pos-1+count; i++)
    {
        self->rgb_data[i*3] = reverse_bits(G);
        self->rgb_data[i*3+1] = reverse_bits(R);
        self->rgb_data[i*3+2] = reverse_bits(B);
    }

    if(update)
    {
        // Вызываем функцию SDL для передачи байтов.
        // Для преобразования байтов в структуры RMT, будет задействована функция транслятор.
        ESP_ERROR_CHECK(rmt_write_sample(RMT_TX_CHANNEL, self->rgb_data, COUNT_LEDS*3, true));
    }

    return mp_const_none;
}

// Функция установки цвета светодиода (или нескольких последовательно)
// Параметры:
// pos      - номер светодиода в цепочке
// color    - устанавливай цвет в виде целого числа, в котором каждые 8 бит - одна цветовая составляющая (0xRRGGBB)
// num      - кол-во светодиодов (начиная с позиции pos) которым будет установлен цвет
// update   - сгенерировать сигналы упрвления цепочной светодиодов
STATIC mp_obj_t ifun_neopixel_set(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_pos,   MP_ARG_REQUIRED | MP_ARG_INT,  {.u_int = 1} },
        { MP_QSTR_R, MP_ARG_REQUIRED | MP_ARG_INT,  {.u_int = 0} },
        { MP_QSTR_G, MP_ARG_REQUIRED | MP_ARG_INT,  {.u_int = 0} },
        { MP_QSTR_B, MP_ARG_REQUIRED | MP_ARG_INT,  {.u_int = 0} },
        { MP_QSTR_num,                     MP_ARG_INT,  {.u_int = 1} },
        { MP_QSTR_update,                  MP_ARG_BOOL, {.u_bool = true} }
    };

    ifun_neopixel_obj_t *self = pos_args[0];

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint8_t pos = args[0].u_int;
    uint8_t R = (uint8_t)args[1].u_int;
    uint8_t G = (uint8_t)args[2].u_int;
    uint8_t B = (uint8_t)args[3].u_int;
    uint8_t count = args[4].u_int;
    bool update = args[5].u_bool;

    return _ifun_neopixel_set(self, pos, R, G, B, count, update);
}
MP_DEFINE_CONST_FUN_OBJ_KW(ifun_neopixel_set_obj, 3, ifun_neopixel_set);


STATIC mp_obj_t ifun_neopixel_setHSB(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {

    const mp_arg_t allowed_args[] = {
        { MP_QSTR_pos,         MP_ARG_REQUIRED | MP_ARG_INT,  { .u_int = 0} },
        { MP_QSTR_hue,         MP_ARG_REQUIRED | MP_ARG_OBJ,  { .u_obj = mp_const_none } },
        { MP_QSTR_saturation,  MP_ARG_REQUIRED | MP_ARG_OBJ,  { .u_obj = mp_const_none } },
        { MP_QSTR_brightness,  MP_ARG_REQUIRED | MP_ARG_OBJ,  { .u_obj = mp_const_none } },
        { MP_QSTR_num,                           MP_ARG_INT,  { .u_int = 1} },
        { MP_QSTR_update,                        MP_ARG_BOOL, { .u_bool = true} },
    };
    ifun_neopixel_obj_t *self = pos_args[0];

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_float_t _hue = mp_obj_get_float(args[1].u_obj);
    mp_float_t _sat = mp_obj_get_float(args[2].u_obj);
    mp_float_t _brightness = mp_obj_get_float(args[3].u_obj);

    uint8_t pos = args[0].u_int;
    uint8_t count = args[4].u_int;
    bool update = args[5].u_bool;

    float red = 0.0;
    float green = 0.0;
    float blue = 0.0;

    if (_sat == 0.0) {
        red = _brightness;
        green = _brightness;
        blue = _brightness;
    }
    else {
        if (_hue >= 360.0) _hue = fmod(_hue, 360);

        int slice = (int)(_hue / 60.0);
        float hue_frac = (_hue / 60.0) - slice;

        float aa = _brightness * (1.0 - _sat);
        float bb = _brightness * (1.0 - _sat * hue_frac);
        float cc = _brightness * (1.0 - _sat * (1.0 - hue_frac));

        switch(slice) {
            case 0:
                red = _brightness;
                green = cc;
                blue = aa;
                break;
            case 1:
                red = bb;
                green = _brightness;
                blue = aa;
                break;
            case 2:
                red = aa;
                green = _brightness;
                blue = cc;
                break;
            case 3:
                red = aa;
                green = bb;
                blue = _brightness;
                break;
            case 4:
                red = cc;
                green = aa;
                blue = _brightness;
                break;
            case 5:
                red = _brightness;
                green = aa;
                blue = bb;
                break;
            default:
                red = 0.0;
                green = 0.0;
                blue = 0.0;
                break;
        }
    }

    return _ifun_neopixel_set(self, pos, (uint8_t)(red * 255.0), (uint8_t)(green * 255.0), (uint8_t)(blue * 255.0), count, update);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(ifun_neopixel_setHSB_obj, 5, ifun_neopixel_setHSB);


//==============================================================================
// Определение таблицы состава класса
// Указываются члены класса
STATIC const mp_rom_map_elem_t ifun_neopixel_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_deinit),     (mp_obj_t)&ifun_neopixel_deinit_obj },
    { MP_ROM_QSTR(MP_QSTR_set),     (mp_obj_t)&ifun_neopixel_set_obj },
    { MP_ROM_QSTR(MP_QSTR_setHSB),     (mp_obj_t)&ifun_neopixel_setHSB_obj }
};
STATIC MP_DEFINE_CONST_DICT(ifun_neopixel_locals_dict, ifun_neopixel_locals_dict_table);


//==============================================================================
// Определение типа класса
const mp_obj_type_t ifun_neopixel_type =
{
    { &mp_type_type },
    .name = MP_QSTR_IFunNeopixel,
    .print = ifun_neopixel_print,
    .make_new = ifun_neopixel_make_new,
    .locals_dict = (mp_obj_dict_t*)&ifun_neopixel_locals_dict
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
