library ieee;
use ieee.std_logic_1164.all;

entity detect_front is
  port( 
    clk : in std_logic;
    reset : in std_logic;
    E : in std_logic;
    ft_mt : out std_logic);
end detect_front;

architecture Behavioral of detect_front is
  type state is (A, B, C);
  signal current_state, next_state : state ;
begin
  process(clk) -- registre : y3adi mn etat l etat
  begin
    if rising_edge(clk) then
      if reset = '1' then 
        current_state <= A;
      else
        current_state <= next_state;
      end if;
    end if;
  end process;
  process(current_state, E)  -- define the next tsate w y5dem l sortie 
  begin
    ft_mt <= '0';
    next_state <= current_state;
    
    case current_state is
      when A =>
        if E = '1' then
          next_state <= B;
        end if;
        
      when B =>
        ft_mt <= '1';
        if E = '1' then
          next_state <= C;
        else
          next_state <= A;
        end if;
        
      when C =>
        if E = '0' then
          next_state <= A;
        end if;
    end case;
  end process;  
end Behavioral; 