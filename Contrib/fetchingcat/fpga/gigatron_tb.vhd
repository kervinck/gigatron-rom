
-- written by Robert Sanchez

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;


entity gigatron_tb is
end gigatron_tb;

architecture Behavioral of gigatron_tb is

    component gigatron
    Port (
         sysclk : in std_logic;
         VGA_R : out std_logic_vector(3 downto 0);
         VGA_G : out std_logic_vector(3 downto 0);
         VGA_B : out std_logic_vector(3 downto 0);
         VGA_HS : out std_logic; 
         VGA_VS : out std_logic;
         LED : out std_logic_vector(7 downto 0);
         BTN : in  std_logic_vector(4 downto 0);
         SW  : in  std_logic_vector(2 downto 0);
         AUD_SD : out std_logic;
         AUD_PWM : out std_logic        
         );
    end component;

    signal sysclk : std_logic := '0';
    signal LED : std_logic_vector(7 downto 0);
    signal BTN : std_logic_vector(4 downto 0);
    signal SW  : std_logic_vector(2 downto 0);
    signal AUD_SD : std_logic;
    signal AUD_PWM : std_logic;
    signal VGA_R : std_logic_vector(3 downto 0);
    signal VGA_G : std_logic_vector(3 downto 0);
    signal VGA_B : std_logic_vector(3 downto 0);
    
    signal VGA_HS : std_logic;
    signal VGA_VS : std_logic;

    constant clk_period : time := 10 ns;

begin

    uut: gigatron port map(sysclk, VGA_R, VGA_G, VGA_B, VGA_HS, VGA_VS, LED, BTN, SW, AUD_SD, AUD_PWM);

    clk_process: process
    begin

    wait for clk_period/2;
    sysclk <= '1';
    wait for clk_period/2;
    sysclk <= '0';

    end process;

end Behavioral;
