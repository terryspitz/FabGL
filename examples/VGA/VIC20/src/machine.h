/*
  Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - www.fabgl.com
  Copyright (c) 2019-2020 Fabrizio Di Vittorio.
  All rights reserved.

  This file is part of FabGL Library.

  FabGL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FabGL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FabGL.  If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once

#include "fabgl.h"

#include "MOS6502.h"
#include "VIA6522.h"
#include "MOS6561.h"


#define DEBUGMACHINE 0


#if DEBUGMACHINE || DEBUG6522 || DEBUG6561
#define DEBUG 1
#endif



/////////////////////////////////////////////////////////////////////////////////////////////
// Machine (Commodore VIC 20)


enum Joy {
  JoyUp,
  JoyDown,
  JoyLeft,
  JoyRight,
  JoyFire,
};


enum RAMExpansionOption {
  RAM_unexp,
  RAM_3K,
  RAM_8K,
  RAM_16K,
  RAM_24K,
  RAM_27K,  // 3K + 24K
  RAM_32K,  // last 8K mapped to A000, not visible to Basic
  RAM_35K,  // as RAM_32K + 3K
};


enum JoyEmu {
  JE_None,
  JE_CursorKeys,
  JE_Mouse,
};



class Machine {

public:

  Machine(fabgl::VGAController * displayController);
  ~Machine();

  void reset();

  int run();

  MOS6522 & VIA1() { return m_VIA1; }
  MOS6522 & VIA2() { return m_VIA2; }
  MOS6561 & VIC()  { return m_VIC; }
  MOS6502 & CPU()  { return m_CPU; }

  void setKeyboard(VirtualKey key, bool down);
  void resetKeyboard();

  void setJoy(Joy joy, bool value) { m_JOY[joy] = value; }
  void resetJoy();
  void setJoyEmu(JoyEmu value) { m_joyEmu = value; }
  JoyEmu joyEmu()              { return m_joyEmu; }

  void loadPRG(char const * filename, bool resetRequired, bool execRun);

  int loadCRT(char const * filename, bool reset, int address = -1);

  void removeCRT();

  uint8_t busRead(int addr);

  uint8_t busReadCharDefs(int addr);
  uint8_t const * busReadVideoP(int addr);
  uint8_t const * busReadColorP(int addr);

  void busWrite(int addr, uint8_t value);

  uint8_t page0Read(int addr)               { return m_RAM1K[addr]; }
  void page0Write(int addr, uint8_t value)  { m_RAM1K[addr] = value; }

  uint8_t page1Read(int addr)               { return m_RAM1K[0x100 + addr]; }
  void page1Write(int addr, uint8_t value)  { m_RAM1K[0x100 + addr] = value; }

  void type(char const * str) { m_typingString = str; }   // TODO: multiple type() calls not possible!!

  void setRAMExpansion(RAMExpansionOption value);
  RAMExpansionOption RAMExpansion() { return m_RAMExpansion; }

private:

  int VICRead(int reg);
  void VICWrite(int reg, int value);

  static void VIA1PortOut(MOS6522 * via, VIAPort port);
  static void VIA1PortIn(MOS6522 * via, VIAPort port);
  static void VIA2PortOut(MOS6522 * via, VIAPort port);
  static void VIA2PortIn(MOS6522 * via, VIAPort port);

  void syncTime();
  void handleCharInjecting();
  void handleMouse();

  // 0: 3K RAM expansion (0x0400 - 0x0fff)
  // 1: 8K RAM expansion (0x2000 - 0x3fff)
  // 2: 8K RAM expansion (0x4000 - 0x5fff)
  // 3: 8K RAM expansion (0x6000 - 0x7fff)
  // 4: 8K RAM expansion (0xA000 - 0xBfff)
  void enableRAMBlock(int block, bool enabled);


  MOS6502               m_CPU;

  // standard RAM
  uint8_t *             m_RAM1K;
  uint8_t *             m_RAM4K;
  uint8_t *             m_RAMColor;

  // expansion RAM
  // 0: 3K (0x0400 - 0x0fff)
  // 1: 8K (0x2000 - 0x3fff)
  // 2: 8K (0x4000 - 0x5fff)
  // 3: 8K (0x6000 - 0x7fff)
  // 4: 8K (0xA000 - 0xBfff)
  uint8_t *             m_expRAM[5];
  RAMExpansionOption    m_RAMExpansion;

  // Cartridges:
  //   block 0 : 0x2000 - 0x3fff
  //   block 1 : 0x4000 - 0x5fff
  //   block 2 : 0x6000 - 0x7fff
  //   block 3 : 0xA000 - 0xbfff
  uint8_t *             m_expROM[4];

  // VIA1 -> NMI, Restore key, joystick
  MOS6522               m_VIA1;

  // VIA2 -> IRQ, keyboard Col (PB0..PB7), Keyboard Row (PA0..PA7), joystick (right)
  MOS6522               m_VIA2;

  // Video Interface
  MOS6561               m_VIC;

  // to store current NMI status (true=active, false=inactive)
  bool                  m_NMI;

  // overflows about every hour
  uint32_t              m_cycle;

  // row x col (1=down, 0 = up)
  uint8_t               m_KBD[8][8];

  // joystick states and emulation
  uint8_t               m_JOY[JoyFire + 1];
  JoyEmu                m_joyEmu;

  // triggered by type() method
  char const *          m_typingString;

  uint32_t              m_lastSyncCycle;
  uint64_t              m_lastSyncTime;   // uS

};



