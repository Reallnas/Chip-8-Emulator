#ifndef CHIP8_H_INCLUDED
#define CHIP8_H_INCLUDED

#include <array>
#include <stack>
#include <vector>

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

class Chip8
{
    public:

        enum State
        {
            LOADING_SUCCESS,
            ERR_FILE_OPENING,
            ERR_FILE_INVALID_SIZE,
            ERR_FILE_READ,
            ERR_WRONG_KEY_PASSED,
            GOOD_KEY_PASSED,
        };
        Chip8();
        ~Chip8();
        void reset();
        void emulateCycle();
        State loadApplication(const char * filename);
        bool needToRedraw();
        const std::array<u8,64*32>& getGFX();
        State setKeyPressed(const char key);
        State setKeyReleased(const char key);
        int countDownDelayTimer();
        int countDownSoundTimer();
    private:
        void loadFontset();
        u8 m_delay_timer = 0;
        u8 m_sound_timer = 0;
        u16 I = 0;
        u16 m_program_counter = 0x200;
        std::array<u8,0x1000> memory {};
        std::array<u8,0x10> V {};
        std::array<u8,64*32> gfx {};
        std::stack<u16,std::vector<u16>> stack {};
        std::array<u8,0x10> m_key_array {};

        bool m_need_to_redraw = false;
};

#endif // CHIP8_H_INCLUDED
