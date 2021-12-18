#include "Chip8.h"

#include <cctype>
#include <cstdio>
#include <chrono>
//#include <fstream>
#include <random>
/*
#include <functional>

struct FunctionInfoBase {
    u16 expected_header;
    std::function<int> handler_callback = {};
    const char* name;
};*/
/*
//#define DEBUG_LOG(...) std::cout << __VA_ARGS__ << std::endl;
//#define DEBUG
#ifdef DEBUG
//#define M_DebugLog( msg )  std::cout << __FILE__ << ":" << __LINE__ << ": " << msg
void DEBUG_LOG(const char* msg)
{
    std::cout << msg << std::endl;
}
//#define DEBUG_LOG(...) std::cout << __VA_ARGS__ << std::endl;
#else
void DEBUG_LOG(const char* msg) {}
//#define DEBUG_LOG(...)
#endif*/

#include <iostream>
void DEBUG_LOG(const char* msg) {}

Chip8::Chip8()
{
    loadFontset();
};
Chip8::~Chip8() = default;

bool Chip8::needToRedraw()
{
    const bool need_to_redraw = m_need_to_redraw;
    m_need_to_redraw = false;
    return need_to_redraw;
}

const std::array<u8,64*32>& Chip8::getGFX()
{
    return gfx;
}

void Chip8::reset()
{
    m_need_to_redraw = false;
    I = 0;
    m_program_counter = 0x200;
    m_delay_timer = 0;
    m_sound_timer = 0;

    V = {};
    m_key_array = {};
    stack = {};
    gfx = {};
    memory = {};

    loadFontset();
}

void Chip8::emulateCycle()
{    
    const u16 opcode = static_cast<u16>(memory[m_program_counter] << 8 | memory[m_program_counter+1]);
    const u16 NNN(opcode & 0x0FFF);
    const u8 X((opcode & 0x0F00) >> 8); //apparently, X is always at the second position
    const u8 Y((opcode & 0x00F0) >> 4); //and Y at the third
    const u8 NN(opcode & 0x00FF);
    switch (opcode & 0xF000)
    {
        case 0x0000:
            switch(opcode & 0x00FF)
            {
                //00EE: Returns from a subroutine.
                case 0x00EE:
                    DEBUG_LOG("Instruction: 00EE");
                    m_program_counter = stack.top();
                    stack.pop();
                    m_program_counter += 2;
                    break;
                case 0x00E0:
                    DEBUG_LOG("Instruction: 00E0");
                    gfx = {};
                    m_need_to_redraw = true;
                    m_program_counter +=2;
                    break;
                default:
                    DEBUG_LOG("Instruction: 0NNN (Unimplemented)");
                    m_program_counter +=2;
                    break;
            }
            break;

        //1NNN: Jumps to address NNN.
        case 0x1000:
            DEBUG_LOG("Instruction: 1NNN");
            m_program_counter = NNN;
            break;

        //2NNN: Calls subroutine at NNN.
        case 0x2000:
            DEBUG_LOG("Instruction: 2NNN");
            stack.push(m_program_counter);
            m_program_counter = NNN;
            break;

        //3XNN: Skips the next instruction if VX equals NN.
        //(Usually the next instruction is a jump to skip a code block)
        case 0x3000:
            DEBUG_LOG("Instruction: 3XNN");
            m_program_counter += 2;
            if (V[X] == NN)
                m_program_counter += 2;
            break;

        //4XNN: Skips the next instruction if VX doesn't equal NN.
        //(Usually the next instruction is a jump to skip a code block)
        case 0x4000:
            DEBUG_LOG("Instruction: 4XNN");
            m_program_counter += 2;
            if (V[X] != NN)
                m_program_counter += 2;
            break;

        //5XY0: Skips the next instruction if VX equals VY.
        //(Usually the next instruction is a jump to skip a code block)
        case 0x5000:
            DEBUG_LOG("Instruction: 5XY0");
            if (V[X] == V[Y])
                m_program_counter += 4;
            else
                m_program_counter += 2;
            break;

        //6XNN: Sets VX to NN.
        case 0x6000:
            DEBUG_LOG("Instruction: 6XNN");
            V[X] = NN;
            m_program_counter +=2;
            break;

        //7XNN: Adds NN to VX. (Carry flag is not changed)
        case 0x7000:
            DEBUG_LOG("Instruction: 7XNN");
            V[X] += NN;
            m_program_counter +=2;
            break;

        case 0x8000:
            switch(opcode & 0x000F)
            {
                //8XY0: Sets VX to the value of VY.
                case 0x0000:
                    DEBUG_LOG("Instruction: 8XY0");
                    V[X] = V[Y];
                    m_program_counter +=2;
                    break;
                //8XY1: Sets VX to VX or VY. (Bitwise OR operation)
                case 0x0001:
                    DEBUG_LOG("Instruction: 8XY1");
                    V[X] |= V[Y];
                    m_program_counter +=2;
                    break;
                //8XY2: Sets VX to VX and VY. (Bitwise AND operation)
                case 0x0002:
                    DEBUG_LOG("Instruction: 8XY2");
                    V[X] &= V[Y];
                    m_program_counter +=2;
                    break;
                //8XY3: Sets VX to VX xor VY.
                case 0x0003:
                    DEBUG_LOG("Instruction: 8XY3");
                    V[X] ^= V[Y];
                    m_program_counter +=2;
                    break;
                //8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
                case 0x0004:
                    DEBUG_LOG("Instruction: 8XY4");
                    if(V[Y] > (0xFF - V[X]))
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    V[X] += V[Y];
                    m_program_counter +=2;
                    break;

                //8XY5: VY is subtracted from VX.
                //VF is set to 0 when there's a borrow, and 1 when there isn't.
                case 0x0005:
                    DEBUG_LOG("Instruction: 8XY5");
                    if(V[Y] > V[X])
                        V[0xF] = 0;
                    else
                        V[0xF] = 1;
                    V[X] -= V[Y];
                    m_program_counter +=2;
                    break;
                //8XY6:	Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
                case 0x0006:
                    DEBUG_LOG("Instruction: 8XY6");
                    V[0xF] = V[X] & 0x1;
                    V[X] >>= 1;
                    m_program_counter +=2;
                    break;
                //8XY7: Sets VX to VY minus VX.
                //VF is set to 0 when there's a borrow, and 1 when there isn't.
                case 0x0007:
                    DEBUG_LOG("Instruction: 8XY7");
                    if(V[X] > V[Y])
                        V[0xF] = 0;
                    else
                        V[0xF] = 1;
                    V[X]= V[Y]-V[X];
                    m_program_counter +=2;
                    break;
                //8XYE:	Stores the most significant bit of VX in VF and then shifts VX to the left by 1
                case 0x000E:
                    DEBUG_LOG("Instruction: 8XYE");
                    V[0xF] = V[X] & 0x8;
                    V[X] <<=  1;
                    m_program_counter +=2;
                    break;
            }
            break;

        //9XY0: Skips the next instruction if VX doesn't equal VY.
        //(Usually the next instruction is a jump to skip a code block)
        case 0x9000:
            DEBUG_LOG("Instruction: 9XY0");
            if (V[X] != V[Y])
                m_program_counter += 4;
            else
                m_program_counter += 2;
            break;

        //ANNN: Sets I to the address NNN.
        case 0xA000:
            DEBUG_LOG("Instruction: ANNN");
            I = NNN;
            m_program_counter += 2;
            break;

        //BNNN: Jumps to the address NNN plus V0.
        case 0xB000:
            DEBUG_LOG("Instruction: BNNN");
            m_program_counter = NNN+V[0];
            break;

        //CXNN: Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
        case 0xC000:
        {
            DEBUG_LOG("Instruction: CXNN");
            static const auto random_seed = static_cast<u32>(std::chrono::system_clock::now().time_since_epoch().count());
            static std::default_random_engine generator (random_seed);

            std::uniform_int_distribution<u8> distribution(0,255);
            const u8 random_number = distribution(generator);
            V[X]=  random_number & NN;
            m_program_counter +=2;
        }
            break;
        //DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
        //Each row of 8 pixels is read as bit-coded starting from memory location I;
        //I value doesn’t change after the execution of this instruction.
        //As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen
        case 0xD000:
        {
            DEBUG_LOG("Instruction: DXYN");
            const u8 height = opcode & 0x000F;
            V[0xF] = 0;
            for (u8 yline = 0; yline < height; yline++)
            {
                const u8 pixel = memory[I + yline];
                for(u8 xline = 0; xline < 8; xline++)
                {
                  //Check if the current evaluated pixel is set to 1
                  if((pixel & (0x80 >> xline)) != 0)
                  {
                    const u8 posX = V[X] + xline;
                    const u8 posY = V[Y] + yline;
                    const std::size_t pixel_index = (posX + (posY * 64)) % (64*32);
                    if(gfx[pixel_index] == 1)
                    {
                      V[0xF] = 1;
                    }
                    gfx[pixel_index] ^= 1;
                  }
                }
            }
            m_need_to_redraw= true;
            m_program_counter +=2;
        }
            break;

        case 0xE000:
            switch(opcode & 0x00FF)
            {
                // EX9E: Skips the next instruction if the key stored in VX is pressed
                // (Usually the next instruction is a jump to skip a code block)
                case 0x009E:
                    DEBUG_LOG("Instruction: EX9E");
                    if(m_key_array[V[X]] != 0)
                        m_program_counter += 4;
                    else
                        m_program_counter += 2;
                    break;
                //EXA1:	Skips the next instruction if the key stored in VX isn't pressed.
                //(Usually the next instruction is a jump to skip a code block)
                case 0x00A1:
                    DEBUG_LOG("Instruction: EXA1");
                    if(m_key_array[V[X]] == 0)
                        m_program_counter += 4;
                    else
                        m_program_counter += 2;
                    break;
            }
            break;

    case 0xF000:
        switch(opcode & 0x00FF)
        {
            //FX07: Sets VX to the value of the delay timer.
            case 0x0007:
                DEBUG_LOG("Instruction: FX07");
                V[X] = m_delay_timer;
                m_program_counter +=2;
                break;
            //FX0A: A key press is awaited, and then stored in VX.
            //(Blocking Operation. All instruction halted until next key event)
            case 0x000A:
            {
                DEBUG_LOG("Instruction: FX0A");
                bool keyPress = false;

                for(u8 i = 0; i < 16; ++i)
                {
                    if(m_key_array[i] != 0)
                    {
                        V[X] = i;
                        keyPress = true;
                    }
                }

                // If we received a keypress, go to the next opcode.
                if(keyPress)
                    m_program_counter +=2;
            }
                break;
            //FX15: Sets the delay timer to VX.
            case 0x0015:
                DEBUG_LOG("Instruction: FX15");
                m_delay_timer = V[X];
                m_program_counter +=2;
                break;
            //FX18: Sets the sound timer to VX.
            case 0x0018:
                DEBUG_LOG("Instruction: FX18");
                m_sound_timer = V[X];
                m_program_counter +=2;
                break;
            //FX1E: Adds VX to I.
            //VF is set to 1 when there is a range overflow (I+VX>0xFFF), and to 0 when there isn't.
            //This is an undocumented feature of the CHIP-8 and used by the Spacefight 2091! game.
            case 0x001E:
                DEBUG_LOG("Instruction: FX1E");
                if(V[X]>(0xFFF-I))
                    V[0xF] = 1;
                else
                    V[0xF] = 0;
                I += V[X];
                m_program_counter +=2;
                break;
            //FX29:	Sets I to the location of the sprite for the character in VX.
            //Characters 0-F (in hexadecimal) are represented by a 4x5 font.
            case 0x0029:
                DEBUG_LOG("Instruction: FX29");
                I = 5 * V[X];
                m_program_counter +=2;
                break;
            //FX33: Stores the binary-coded decimal representation of VX,
            //with the most significant of three digits at the address in I,
            //the middle digit at I plus 1, and the least significant digit at I plus 2.
            //(In other words, take the decimal representation of VX,
            //place the hundreds digit in memory at location in I,
            //the tens digit at location I+1, and the ones digit at location I+2.)
            case 0x0033:
                DEBUG_LOG("Instruction: FX33");
                memory[I]     = V[X] / 100;
                memory[I + 1] = (V[X] / 10) % 10;
                memory[I + 2] = (V[X] % 100) % 10;
                m_program_counter +=2;
                break;
            //FX55: Stores V0 to VX (including VX) in memory starting at address I.
            //The offset from I is increased by 1 for each value written, but I itself is left unmodified.
            case 0x0055:
                DEBUG_LOG("Instruction: FX55");
                for(unsigned int i = 0;i<=X;++i)
                {
                    memory[I+i] = V[i];
                }
                m_program_counter +=2;
                break;
            //FX65: Fills V0 to VX (including VX) with values from memory starting at address I.
            //The offset from I is increased by 1 for each value written, but I itself is left unmodified.
            case 0x0065:
                DEBUG_LOG("Instruction: FX65");
                for(unsigned int i = 0;i<=X;++i)
                {
                    V[i] = memory[I+i];
                }
                m_program_counter +=2;
                break;
        }
        break;
        default:
            std::cout << "Unknown opcode: "<< opcode << std::endl;
            m_program_counter +=2;
            break;
    }
}

void Chip8::loadFontset() {
    constexpr u8 chip8_fontset[80] =
    {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for(std::size_t i = 0; i < 80; ++i)
    {
        memory[i] = chip8_fontset[i];
    }
}
Chip8::State Chip8::loadApplication(const char* filename)
{

    /*std::ifstream file(filename,std::ifstream::binary);

    // If we couldn't open the input file stream for reading
    if (!file)
    {
        std::cout << "File opening error !" << std::endl;
        return -1;
    }
      // get length of file:
      file.seekg (0, file.end);
      int length = file.tellg();
      file.seekg (0, file.beg);

      char * buffer = new char [length];

      std::cout << "Reading " << length << " characters... ";
      // read data as a block:
      file.read (buffer,length);

      if (file)
        std::cout << "all characters read successfully.";
      else
        std::cout << "error: only " << is.gcount() << " could be read";
      file.close();

        u8 strInput;
        file.get(memory.data()+0x200,length);
*/
    FILE* file = fopen(filename, "rb");
    if (!file)
    {
        std::cout << "File opening error !" << std::endl;
        fclose(file);
        return ERR_FILE_OPENING;
    }
    fseek(file,0,SEEK_END);
    const auto fileSize = static_cast<size_t>(ftell(file));
    rewind(file);
    if (fileSize > memory.size()-512) {
        std::cout << "Invalid file size: "<< fileSize << std::endl;
        fclose(file);
        return ERR_FILE_INVALID_SIZE;
    }
    const size_t bytesRead = fread(memory.data()+0x200,1,fileSize,file);
    if (bytesRead != fileSize) {
         std::cout << "Invalid number of bytes read : "<< bytesRead << std::endl;
         fclose(file);
         return ERR_FILE_READ;
    }
    /*for(size_t i=0; i < fileSize; ++i)
    {
        std::cout << static_cast<int>(memory[i+512]);
    }*/
    fclose(file);
    return LOADING_SUCCESS;
}

Chip8::State Chip8::setKeyPressed(const char key)
{
    //TODO: maybe change with cstdlib strol
    if(isxdigit(key)){
        if(isdigit(key))
        {
            const u8 index = static_cast<u8>(key-0x30);
            m_key_array[index] = 1;
        }
        else
        {
            const u8 index = static_cast<u8>(key-0x37);
            m_key_array[index] = 1;
        }
        return GOOD_KEY_PASSED;
    }
    else
    {
        return ERR_WRONG_KEY_PASSED;
    }
}

Chip8::State Chip8::setKeyReleased(const char key)
{
    if(isxdigit(key)){
        if(isdigit(key))
        {
            const u8 index = static_cast<u8>(key-0x30);
            m_key_array[index] = 0;
        }
        else
        {
            const u8 index = static_cast<u8>(key-0x37);
            m_key_array[index] = 0;
        }
        return GOOD_KEY_PASSED;
    }
    else
    {
        return ERR_WRONG_KEY_PASSED;
    }
}

int Chip8::countDownDelayTimer()
{
    if (m_delay_timer > 0)
    {
        --m_delay_timer;
    }
    return m_delay_timer;
}

int Chip8::countDownSoundTimer()
{
    if (m_sound_timer > 0)
    {
        --m_sound_timer;
    }
    return m_sound_timer;
}
