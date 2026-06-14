-------------------------------------------------------------------------------
-- Top-level processor component
--
-- Ports:
--   - clk [in]  : clock signal
--   - rst [in]  : reset signal
--
--   - ram_addr [out] : memory address
--   - ram_din  [out] : data input to memory
--   - ram_dout [in]  : data output from memory
--   - ram_we   [out] : write enable signal for memory
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity proc is
  port ( clk : in  std_logic;
         rst : in  std_logic;
         
         ram_addr : out std_logic_vector(15 downto 0);
         ram_din  : out std_logic_vector(15 downto 0);
         ram_dout : in  std_logic_vector(15 downto 0);
         ram_we   : out std_logic );
end entity;

architecture arch of proc is
  component instr_reg_dummy is
   port ( clk : in  std_logic;
          ce  : in  std_logic;
          rst : in  std_logic;

          instr : in  std_logic_vector(15 downto 0);
          cond  : out std_logic_vector(3 downto 0);
          op    : out std_logic_vector(3 downto 0);
          updt  : out std_logic;
          imm   : out std_logic;
          val   : out std_logic_vector(5 downto 0) );
  end component;

  component status_reg_dummy is
   port ( clk : in  std_logic;
          ce  : in  std_logic;
          rst : in  std_logic;

          i : in  std_logic_vector(3 downto 0);
          o : out std_logic_vector(3 downto 0) );
  end component;

  component reg_file_dummy is
    port ( clk : in  std_logic;
           rst : in  std_logic;

           acc_out : out std_logic_vector(15 downto 0);
           acc_ce  : in  std_logic;

           pc_out : out std_logic_vector(15 downto 0);
           pc_ce  : in  std_logic;
           rpc_ce : in  std_logic;

           rx_num : in  std_logic_vector(5 downto 0);
           rx_out : out std_logic_vector(15 downto 0);
           rx_ce  : in  std_logic;

           din : in  std_logic_vector(15 downto 0) );
  end component;

  component alu_dummy is
    port ( op : in  std_logic_vector(3 downto 0);
           i1 : in  std_logic_vector(15 downto 0);
           i2 : in  std_logic_vector(15 downto 0);
           o  : out std_logic_vector(15 downto 0);
           st : out std_logic_vector(3 downto 0) );
  end component;

  component control_dummy is
    port ( clk : in  std_logic;
           rst : in  std_logic;

           status     : in  std_logic_vector(3 downto 0);
           instr_cond : in  std_logic_vector(3 downto 0);
           instr_op   : in  std_logic_vector(3 downto 0);
           instr_updt : in  std_logic;

           instr_ce  : out std_logic;
           status_ce : out std_logic;
           acc_ce    : out std_logic;
           pc_ce     : out std_logic;
           rpc_ce    : out std_logic;
           rx_ce     : out std_logic;

           ram_we : out std_logic;

           sel_ram_addr : out std_logic;
           sel_op1      : out std_logic;
           sel_rf_din   : out std_logic_vector(1 downto 0) );
  end component;
begin
end architecture;
