library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity tb_Gestion_Parking is

end tb_Gestion_Parking;

architecture Behavioral of tb_Gestion_Parking is
    -- 1. Signaux pour relier au composant (mêmes noms que les ports)
    signal clk           : std_logic := '0';
    signal reset         : std_logic := '0';
    signal capt_entree   : std_logic := '0';
    signal capt_sortie   : std_logic := '0';
    signal ld_reg        : std_logic := '0';
    signal Nb_max        : std_logic_vector(3 downto 0) := "0000";
    signal Complet       : std_logic;
    signal nb_places_dispos : std_logic_vector(3 downto 0);

    -- 2. Définition de la période d'horloge (ex: 10ns pour 100MHz)
    constant clk_period : time := 10 ns;

    component Gestion_Parking
        Port ( clk : in std_logic; reset : in std_logic; capt_entree : in std_logic; capt_sortie : in std_logic; ld_reg : in std_logic; Nb_max : in std_logic_vector(3 downto 0); Complet : out std_logic; nb_places_dispos : out std_logic_vector(3 downto 0) );
    end component;

begin
    -- 3. Instanciation du module à tester (UUT : Unit Under Test)
    UUT: Gestion_Parking
        port map (
            clk => clk, reset => reset, capt_entree => capt_entree,
            capt_sortie => capt_sortie, ld_reg => ld_reg,
            Nb_max => Nb_max, Complet => Complet,
            nb_places_dispos => nb_places_dispos);

    -- 4. Génération de l'horloge
    clk_process : process
    begin
        clk <= '0'; wait for clk_period/2;
        clk <= '1'; wait for clk_period/2;
    end process;

    -- 5. Le scénario de test 
    stim_proc: process
    begin		
        -- Reset du système
        reset <= '1'; wait for 20 ns;
        reset <= '0'; wait for 20 ns;

        -- Configurer le parking à 3 places max
        Nb_max <= "0011"; 
        ld_reg <= '1'; wait for clk_period;
        ld_reg <= '0'; wait for 20 ns;

        -- Simulation : Une voiture arrive (front sur capt_entree)
        capt_entree <= '1'; wait for 40 ns; -- On reste sur le capteur
        capt_entree <= '0'; wait for 40 ns; -- Elle est passée

        -- Une deuxième voiture arrive
        capt_entree <= '1'; wait for 20 ns;
        capt_entree <= '0'; wait for 20 ns;

        -- Une troisième voiture arrive (Le parking devrait être plein !)
        capt_entree <= '1'; wait for 20 ns;
        capt_entree <= '0'; wait for 20 ns;

        -- Une 4èmme voiture arrive (Le parking est déjà plein ! : blocage compteur)
        capt_entree <= '1'; wait for 20 ns;
        capt_entree <= '0'; wait for 20 ns;
        
        -- Une voiture sort
        capt_sortie <= '1'; wait for 20 ns;
        capt_sortie <= '0'; wait for 20 ns;
        
        -- Une voiture sort
        capt_sortie <= '1'; wait for 20 ns;
        capt_sortie <= '0'; wait for 20 ns;       

        -- Une voiture sort
        capt_sortie <= '1'; wait for 20 ns;
        capt_sortie <= '0'; wait for 20 ns;
        
        -- Une voiture sort
        capt_sortie <= '1'; wait for 20 ns;
        capt_sortie <= '0';        
        wait; -- Fin de la simulation
    end process;

end Behavioral;
