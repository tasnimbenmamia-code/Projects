library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity Registre is
  port (
    clk : in STD_LOGIC;
    reset : in STD_LOGIC;
    load : in STD_LOGIC;
    d_in : in STD_LOGIC_VECTOR (3 downto 0);
    d_out : out STD_LOGIC_VECTOR (3 downto 0));
end Registre;

architecture Behavioral of Registre is
begin
  process(clk, reset)
  begin
    if reset = '1' then 
      d_out <= (others => '0'); -- "1111" if we want led complet off initially
    elsif rising_edge(clk) then
      if load = '1' then
        d_out <= d_in;
        
        assert (d_in /= "0000")
        report "Warning: parking capacity is 0"
        severity warning;
      end if;
    end if;
  end process;
end Behavioral;  