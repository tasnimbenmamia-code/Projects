library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity Soustracteur is Port (
  A : in std_logic_vector(3 downto 0);
  B : in std_logic_vector(3 downto 0);
  result : out std_logic_vector(3 downto 0));
end Soustracteur;

architecture Behavioral of Soustracteur is
begin
  process(A, B)
  begin
    result <= std_logic_vector(unsigned(B) - unsigned(A));
  end process;
end Behavioral;