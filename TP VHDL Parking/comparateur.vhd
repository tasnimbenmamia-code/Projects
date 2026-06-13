library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity Comparateur is
  Port (
    A : in std_logic_vector(3 downto 0);
    B : in std_logic_vector(3 downto 0);
    sup_egal : out std_logic);
end Comparateur;

architecture Dataflow of Comparateur is 
begin
  sup_egal <= '1' when (unsigned(A) >= unsigned(B))
                       else '0';
end Dataflow;                       