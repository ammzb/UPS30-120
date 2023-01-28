----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    11:56:42 09/03/2012 
-- Design Name: 
-- Module Name:    test1 - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity test1 is
	port (
		CPLD_clk_dsp1: in  STD_LOGIC;
		
		PWM_R_up_dsp1: in  STD_LOGIC;
		PWM_R_down_dsp1: in  STD_LOGIC;
		PWM_S_up_dsp1: in  STD_LOGIC;
		PWM_S_down_dsp1: in  STD_LOGIC;
		PWM_T_up_dsp1: in  STD_LOGIC;
		PWM_T_down_dsp1: in  STD_LOGIC;
		PWM_leg4_up_dsp1: in  STD_LOGIC;
		PWM_leg4_down_dsp1: in  STD_LOGIC;
		PWM_inv_RST_enable_dsp1: in  STD_LOGIC;
		PWM_inv_leg4_enable_dsp1: in  STD_LOGIC;
		
		STS_byepass_Command_L123: in  STD_LOGIC;
		STS_byepass_nEnable: in  STD_LOGIC;
		STS_byepass_nc: in  STD_LOGIC;
		STS_INV_Command_L123: in  STD_LOGIC;
		STS_INV_nEnable: in  STD_LOGIC;
		STS_INV_nc: in  STD_LOGIC;

		cpld_to_STS_in_pulse_IN_1: out  STD_LOGIC;
		cpld_to_STS_in_pulse_IN_2: out  STD_LOGIC;
		cpld_to_STS_in_pulse_IN_3: out  STD_LOGIC;
		cpld_to_STS_in_pulse_INV_1: out  STD_LOGIC;
		cpld_to_STS_in_pulse_INV_2: out  STD_LOGIC;
		cpld_to_STS_in_pulse_INV_3: out  STD_LOGIC;

		CPLD_clk_dsp2: in  STD_LOGIC;
		
		PWM_R_up_dsp2: in  STD_LOGIC;
		PWM_R_down_dsp2: in  STD_LOGIC;
		PWM_S_up_dsp2: in  STD_LOGIC;
		PWM_S_down_dsp2: in  STD_LOGIC;
		PWM_T_up_dsp2: in  STD_LOGIC;
		PWM_T_down_dsp2: in  STD_LOGIC;
		PWM_Batt_leg4_up_dsp2: in  STD_LOGIC;
		PWM_Batt_leg4_down_dsp2: in  STD_LOGIC;
		PWM_RST_enable_dsp2: in  STD_LOGIC;
		PWM_Batt_leg4_enable_dsp2: in  STD_LOGIC;

		inv_R_PWM1: out  STD_LOGIC;
		inv_R_PWM2: out  STD_LOGIC;
		inv_S_PWM1: out  STD_LOGIC;
		inv_S_PWM2: out  STD_LOGIC;
		inv_T_PWM1: out  STD_LOGIC;
		inv_T_PWM2: out  STD_LOGIC;
		inv_4_PWM1: out  STD_LOGIC;
		inv_4_PWM2: out  STD_LOGIC;

		Charger_R_PWM1: out  STD_LOGIC;
		Charger_R_PWM2: out  STD_LOGIC;
		Charger_S_PWM1: out  STD_LOGIC;
		Charger_S_PWM2: out  STD_LOGIC;
		Charger_T_PWM1: out  STD_LOGIC;
		Charger_T_PWM2: out  STD_LOGIC;
		Charger_4_PWM1: out  STD_LOGIC;
		Charger_4_PWM2: out  STD_LOGIC;
		
		test_85 : out  STD_LOGIC;
		CPLD_59 : out  STD_LOGIC;
		clk_30mhz : in  STD_LOGIC
			);
end test1;

architecture Behavioral of test1 is

	COMPONENT pwm_check
	PORT(
		clk_1060n : IN std_logic;
		pwm_check_up : IN std_logic; 
		pwm_check_down : IN std_logic;		
		pwm_ok : OUT std_logic
		);
	END COMPONENT;
	
signal pwm_ok_dsp1 :STD_LOGIC;	
signal pwm_ok_dsp2 :STD_LOGIC;
signal pwm_ok_dsp2_leg4 :STD_LOGIC;
signal clk_1060n :STD_LOGIC;
signal STS_firing :STD_LOGIC;

signal inv_R_PWM1_temp:   STD_LOGIC;
signal inv_R_PWM2_temp:   STD_LOGIC;
signal inv_S_PWM1_temp:   STD_LOGIC;
signal inv_S_PWM2_temp:   STD_LOGIC;
signal inv_T_PWM1_temp:   STD_LOGIC;
signal inv_T_PWM2_temp:   STD_LOGIC;
signal inv_4_PWM1_temp:   STD_LOGIC;
signal inv_4_PWM2_temp:   STD_LOGIC;
signal Charger_R_PWM1_temp:   STD_LOGIC;
signal Charger_R_PWM2_temp:   STD_LOGIC;
signal Charger_S_PWM1_temp:   STD_LOGIC;
signal Charger_S_PWM2_temp:   STD_LOGIC;
signal Charger_T_PWM1_temp:   STD_LOGIC;
signal Charger_T_PWM2_temp:   STD_LOGIC;
signal Charger_4_PWM1_temp:   STD_LOGIC;
signal Charger_4_PWM2_temp:   STD_LOGIC;
		
begin

------------------------------------------------------------------------
clk_process: process(clk_30mhz)

variable counter: STD_LOGIC_VECTOR(7 downto 0);
begin
	if rising_edge(clk_30mhz) then
		counter:=counter + 1;
				end if;
				clk_1060n <= counter(7);
end process;	--clk_process	

test_85 <= pwm_ok_dsp1;
CPLD_59 <= pwm_ok_dsp1;
------------------------------------------------------------------------
--check PWM
--	Inst_pwm_check1: pwm_check PORT MAP(
--		clk_1060n => clk_1060n,
--		pwm_check_up => PWM_R_up_dsp1,
--		pwm_check_down => PWM_R_down_dsp1,
--		pwm_ok => pwm_ok_dsp1
--	);
--
--	Inst_pwm_check2: pwm_check PORT MAP(
--		clk_1060n => clk_1060n,
--		pwm_check_up => PWM_T_up_dsp2,
--		pwm_check_down => PWM_T_down_dsp2,
--		pwm_ok => pwm_ok_dsp2
--	);


--	Inst_pwm_check2_leg: pwm_check PORT MAP(
--		clk_1060n => clk_1060n,
--		pwm_check_up => PWM_Batt_leg4_up_dsp2,
--		pwm_check_down => PWM_Batt_leg4_down_dsp2,
--		pwm_ok => pwm_ok_dsp2_leg4
--	);

pwm_ok_dsp2 <= '1';
pwm_ok_dsp1 <= '1';
pwm_ok_dsp2_leg4 <= '1';

------------------------------------------------------------------------
		inv_R_PWM1_temp <= PWM_R_up_dsp1 		 when PWM_R_down_dsp1='1'    and PWM_inv_RST_enable_dsp1 = '0' and pwm_ok_dsp1 = '1' else '1';
		inv_R_PWM2_temp <= PWM_R_down_dsp1 	 when PWM_R_up_dsp1='1' 	  and PWM_inv_RST_enable_dsp1 = '0' and pwm_ok_dsp1 = '1' else '1';
		inv_S_PWM1_temp <= PWM_S_up_dsp1 	    when PWM_S_down_dsp1='1'    and PWM_inv_RST_enable_dsp1 = '0' and pwm_ok_dsp1 = '1' else '1';
		inv_S_PWM2_temp <= PWM_S_down_dsp1	    when PWM_S_up_dsp1='1'      and PWM_inv_RST_enable_dsp1 = '0' and pwm_ok_dsp1 = '1' else '1';
		inv_T_PWM1_temp <= PWM_T_up_dsp1 		 when PWM_T_down_dsp1='1'    and PWM_inv_RST_enable_dsp1 = '0' and pwm_ok_dsp1 = '1' else '1';
		inv_T_PWM2_temp <= PWM_T_down_dsp1 	 when PWM_T_up_dsp1='1'      and PWM_inv_RST_enable_dsp1 = '0' and pwm_ok_dsp1 = '1' else '1';

--		inv_4_PWM1 <= PWM_leg4_up_dsp1	 when PWM_leg4_down_dsp1='0' and PWM_inv_leg4_enable_dsp1 = '0' else '0';
--		inv_4_PWM2 <= PWM_leg4_down_dsp1  when PWM_leg4_up_dsp1='0'   and PWM_inv_leg4_enable_dsp1 = '0' else '0';
		inv_4_PWM1_temp <='1';
		inv_4_PWM1_temp <='1';
-- change to STS firing
		STS_firing<= not PWM_inv_leg4_enable_dsp1;
					
		Charger_R_PWM1_temp <= PWM_R_up_dsp2 	 when PWM_R_down_dsp2='1' and PWM_RST_enable_dsp2 = '0' and pwm_ok_dsp2 = '1'  else '1';
		Charger_R_PWM2_temp <= PWM_R_down_dsp2 when PWM_R_up_dsp2='1'   and PWM_RST_enable_dsp2 = '0' and pwm_ok_dsp2 = '1'  else '1';
		Charger_S_PWM1_temp <= PWM_S_up_dsp2 	 when PWM_S_down_dsp2='1' and PWM_RST_enable_dsp2 = '0' and pwm_ok_dsp2 = '1'  else '1';
		Charger_S_PWM2_temp <= PWM_S_down_dsp2 when PWM_S_up_dsp2='1'   and PWM_RST_enable_dsp2 = '0' and pwm_ok_dsp2 = '1'  else '1';
		Charger_T_PWM1_temp <= PWM_T_up_dsp2	 when PWM_T_down_dsp2='1' and PWM_RST_enable_dsp2 = '0' and pwm_ok_dsp2 = '1'  else '1';
		Charger_T_PWM2_temp <= PWM_T_down_dsp2 when PWM_T_up_dsp2='1'   and PWM_RST_enable_dsp2 = '0' and pwm_ok_dsp2 = '1'  else '1';
		Charger_4_PWM1_temp <= PWM_Batt_leg4_up_dsp2 	when PWM_Batt_leg4_down_dsp2='1' and PWM_Batt_leg4_enable_dsp2 = '0' and pwm_ok_dsp2_leg4 = '1' else '1';
		Charger_4_PWM2_temp <= PWM_Batt_leg4_down_dsp2 when PWM_Batt_leg4_up_dsp2='1'   and PWM_Batt_leg4_enable_dsp2 = '0' and pwm_ok_dsp2_leg4 = '1' else '1';
		
--		cpld_to_STS_in_pulse_IN_1  <= STS_byepass_Command_L123 when STS_INV_Command_L123 = '0' 	 and STS_byepass_nEnable = '0' else '0';
--		cpld_to_STS_in_pulse_IN_2  <= STS_byepass_Command_L123 when STS_INV_Command_L123 = '0' 	 and STS_byepass_nEnable = '0' else '0';
--		cpld_to_STS_in_pulse_IN_3  <= STS_byepass_Command_L123 when STS_INV_Command_L123 = '0' 	 and STS_byepass_nEnable = '0' else '0';
--		cpld_to_STS_in_pulse_INV_1 <= STS_INV_Command_L123 	 when STS_byepass_Command_L123 = '0' and STS_INV_nEnable = '0' 	  else '0';
--		cpld_to_STS_in_pulse_INV_2 <= STS_INV_Command_L123 	 when STS_byepass_Command_L123 = '0' and STS_INV_nEnable = '0'     else '0';
--		cpld_to_STS_in_pulse_INV_3 <= STS_INV_Command_L123 	 when STS_byepass_Command_L123 = '0' and STS_INV_nEnable = '0'     else '0';
		
		cpld_to_STS_in_pulse_IN_1  <= STS_firing   when STS_INV_nEnable = '1' 	 and STS_byepass_nEnable = '0' else '0';
		cpld_to_STS_in_pulse_IN_2  <= STS_firing   when STS_INV_nEnable = '1' 	 and STS_byepass_nEnable = '0' else '0';
		cpld_to_STS_in_pulse_IN_3  <= STS_firing   when STS_INV_nEnable = '1' 	 and STS_byepass_nEnable = '0' else '0';
		cpld_to_STS_in_pulse_INV_1 <= STS_firing 	 when STS_byepass_nEnable = '1' and STS_INV_nEnable = '0' 	  else '0';
		cpld_to_STS_in_pulse_INV_2 <= STS_firing 	 when STS_byepass_nEnable = '1' and STS_INV_nEnable = '0'     else '0';
		cpld_to_STS_in_pulse_INV_3 <= STS_firing 	 when STS_byepass_nEnable = '1' and STS_INV_nEnable = '0'     else '0';


--- haj mirza bayat demand

 inv_R_PWM1  <=  inv_R_PWM1_temp;
 inv_R_PWM2  <=  inv_R_PWM2_temp;
 inv_S_PWM1  <=  inv_S_PWM1_temp;
 inv_S_PWM2  <=  inv_S_PWM2_temp;
 inv_T_PWM1  <=  inv_T_PWM1_temp;
 inv_T_PWM2  <=  inv_T_PWM2_temp;
 inv_4_PWM1  <=  inv_4_PWM1_temp;
 inv_4_PWM2  <=  inv_4_PWM2_temp;
 Charger_R_PWM1  <=  Charger_R_PWM1_temp;
 Charger_R_PWM2  <=  Charger_R_PWM2_temp;
 Charger_S_PWM1  <=  Charger_S_PWM1_temp;
 Charger_S_PWM2  <=  Charger_S_PWM2_temp;
 Charger_T_PWM1  <=  Charger_T_PWM1_temp;
 Charger_T_PWM2  <=  Charger_T_PWM2_temp;
 Charger_4_PWM1  <=  Charger_4_PWM1_temp;
 Charger_4_PWM2  <=  Charger_4_PWM2_temp;
end Behavioral;

