library IEEE;
use IEEE.std_logic_1164.all;
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL; 

entity Compteur is 
  Port(
     clk : in STD_LOGIC;
     reset : in STD_LOGIC;
     up : in STD_LOGIC;
     down : in STD_LOGIC;
     count : out STD_LOGIC_VECTOR (3 downto 0));
end Compteur;
architecture Behavioral of Compteur is
  signal temp_count : unsigned(3 downto 0) := "0000";
begin
  process(clk, reset)
  begin
    if reset = '1' then
      temp_count <= (others => '0');
    elsif rising_edge(clk) then       
      if up = '1' then 
        temp_count <= temp_count + 1;
      elsif down = '1' then
        temp_count <= temp_count - 1;
      end if;
    end if;
  end process;
  count <= std_logic_vector(temp_count);
end Behavioral;