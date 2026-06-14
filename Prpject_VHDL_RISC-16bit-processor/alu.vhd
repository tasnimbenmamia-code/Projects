library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all; -- Utilisation de numeric_std, plus moderne que std_logic_unsigned

entity alu is
  port ( 
    op : in  std_logic_vector(3 downto 0); -- Opcode
    i1 : in  std_logic_vector(15 downto 0); -- Opérande 1 (souvent Acc)
    i2 : in  std_logic_vector(15 downto 0); -- Opérande 2
    o  : out std_logic_vector(15 downto 0); -- Résultat
    st : out std_logic_vector(3 downto 0)  -- Statut [Z, N, C, V]
  );
end entity;

architecture arch of alu is
begin
    process(op, i1, i2)
        variable res_16 : std_logic_vector(15 downto 0);
        variable res_17 : unsigned(16 downto 0); -- 17 bits pour détecter le Carry
        variable v_flag : std_logic := '0';
        variable c_flag : std_logic := '0';
    begin
        -- Valeurs par défaut
        v_flag := '0';
        c_flag := '0';
        res_16 := (others => '0');

        case op is
            -- AND
            when "0000" => 
                res_16 := i1 and i2;

            -- OR
            when "0001" => 
                res_16 := i1 or i2;

            -- XOR
            when "0010" => 
                res_16 := i1 xor i2;

            -- NOT
            when "0011" => 
                res_16 := not i1;

            -- ADD (Arithmétique avec C et V)
            when "0100" => 
                res_17 := unsigned('0' & i1) + unsigned('0' & i2);
                res_16 := std_logic_vector(res_17(15 downto 0));
                c_flag := res_17(16); -- La retenue
                -- Overflow : Pos+Pos=Neg ou Neg+Neg=Pos
                if ((i1(15)='0' and i2(15)='0' and res_16(15)='1') or
                    (i1(15)='1' and i2(15)='1' and res_16(15)='0')) then
                    v_flag := '1';
                end if;

            -- SUB (Arithmétique avec C et V)
            when "0101" => 
                res_17 := unsigned('0' & i1) - unsigned('0' & i2);
                res_16 := std_logic_vector(res_17(15 downto 0));
                c_flag := res_17(16); -- Emprunt (Borrow)
                -- Overflow : Pos-Neg=Neg ou Neg-Pos=Pos
                if ((i1(15)='0' and i2(15)='1' and res_16(15)='1') or
                    (i1(15)='1' and i2(15)='0' and res_16(15)='0')) then
                    v_flag := '1';
                end if;

            -- LSL (Logical Shift Left)
            when "0110" =>
                c_flag := i1(15); -- Le bit qui sort devient le Carry
                res_16 := i1(14 downto 0) & '0';
                v_flag := i1(15) xor res_16(15); -- V=1 si le signe change

            -- LSR (Logical Shift Right)
            when "0111" =>
                c_flag := i1(0); -- Le bit qui sort à droite
                res_16 := '0' & i1(15 downto 1);

            -- MTA / LDA / MTR (Transferts simples)
            -- Ici on recopie juste i2 vers la sortie
            when others => 
                res_16 := i2; 
        end case;

        -- Affectation de la sortie résultat
        o <= res_16;

        -- Mise à jour du vecteur statut [Z, N, C, V]
        -- st(3) : Z (Zéro)
        if res_16 = x"0000" then st(3) <= '1'; else st(3) <= '0'; end if;
        
        -- st(2) : N (Négatif)
        st(2) <= res_16(15);
        
        -- st(1) : C (Carry)
        st(1) <= c_flag;
        
        -- st(0) : V (Overflow)
        st(0) <= v_flag;

    end process;
end architecture;