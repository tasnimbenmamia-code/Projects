library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL; 

entity Gestion_Parking is
  Port( 
    clk : in std_logic ;
    reset : in std_logic ; 
    capt_entree : in std_logic;
    capt_sortie : in std_logic;
    ld_reg : in std_logic;
    Nb_max : in std_logic_vector(3 downto 0);
    Complet : out std_logic;
    nb_places_dispos : out std_logic_vector(3 downto 0));
end Gestion_Parking;

architecture Structural of Gestion_Parking is
	signal up : std_logic := '0';
    signal down : std_logic := '0';
    signal up_autorise : std_logic := '0';
    signal down_autorise : std_logic := '0';
    signal comptage : std_logic_vector(3 downto 0):= "0000";
    signal val_max : std_logic_vector(3 downto 0):= "1111";

	component detect_front
        Port ( clk, reset, E : in std_logic; ft_mt : out std_logic );
    end component;
    
    component compteur
        Port ( clk, reset, up, down : in std_logic; count : out std_logic_vector(3 downto 0) );
    end component;

    component Registre
        Port ( clk, reset, load : in std_logic; d_in : in STD_LOGIC_VECTOR (3 downto 0); d_out : out STD_LOGIC_VECTOR (3 downto 0) );
    end component;	 

    component comparateur
        Port ( A : in std_logic_vector(3 downto 0); B : in std_logic_vector(3 downto 0); sup_egal : out std_logic );
    end component;	

    component Soustracteur
        Port ( A : in std_logic_vector(3 downto 0); B : in std_logic_vector(3 downto 0); result : out std_logic_vector(3 downto 0) );
    end component;

begin
    up_autorise <= up when (unsigned(comptage) < unsigned(val_max)) else '0';
    down_autorise <= down when (unsigned(comptage) > 0) else '0';
	Detect_front_1: detect_front
    	port map (clk => clk, reset => reset, E => capt_entree, ft_mt => up);
        
	Detect_front_2: detect_front
    	port map (clk => clk, reset => reset, E => capt_sortie, ft_mt => down);
  
    --up_autorise <= (up and (not sig_complet));
    --Complet <= sig_complet;
	u_Compteur: compteur
    	port map (clk => clk, reset => reset, up => up_autorise, down => down_autorise, count => comptage);
        
	u_registre: Registre
    	port map (clk => clk, reset => reset, load => ld_reg, d_in => Nb_max, d_out => val_max);
        
	u_Comparateur: comparateur
    	port map (A => comptage, B => val_max, sup_egal => complet);

	u_Soustracteur: Soustracteur
    	port map (A => comptage, B => val_max, result => nb_places_dispos);
        
end Structural;        