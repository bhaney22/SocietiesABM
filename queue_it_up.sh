#Runs everything. 30 runs each of all 1,3,5,7 r/t , with and without stock, with and without DevDevs. Should be between 6,000 and 9,000 computing hours.

cd PBS_scripts/WithDevDev/1

for i in {1..30}
do
	echo "Queueing run #$i"
	qsub 1_tool.pbs
	qsub 1_tool_nosto.pbs
    	qsub 1_tool_stock.pbs
    	qsub 1_machine.pbs
	qsub 1_machine_nosto.pbs
    	qsub 1_machine_stock.pbs
    	qsub 1_factory.pbs
	qsub 1_factory_nosto.pbs
    	qsub 1_factory_stock.pbs
    	qsub 1_Industry.pbs
	qsub 1_Industry_nosto.pbs
    	qsub 1_Industry_stock.pbs
done

cd ../3
for i in {1..30}
do
	echo "Queueing run #$i"
	qsub 3_tool.pbs
	qsub 3_tool_nosto.pbs
    	qsub 3_tool_stock.pbs
    	qsub 3_machine.pbs
	qsub 3_machine_nosto.pbs
    	qsub 3_machine_stock.pbs
    	qsub 3_factory.pbs
	qsub 3_factory_nosto.pbs
    	qsub 3_factory_stock.pbs
    	qsub 3_Industry.pbs
	qsub 3_Industry_nosto.pbs
    	qsub 3_Industry_stock.pbs
done

cd ../5
for i in {1..30}
do
	echo "Queueing run #$i"
    	qsub 5_none.pbs
	qsub 5_none_nosto.pbs
    	qsub 5_none_stock.pbs
	qsub 5_tool.pbs
	qsub 5_tool_nosto.pbs
    	qsub 5_tool_stock.pbs
    	qsub 5_machine.pbs
	qsub 5_machine_nosto.pbs
    	qsub 5_machine_stock.pbs
    	qsub 5_factory.pbs
	qsub 5_factory_nosto.pbs
    	qsub 5_factory_stock.pbs
    	qsub 5_Industry.pbs
	qsub 5_Industry_nosto.pbs
    	qsub 5_Industry_stock.pbs
done

cd ../7
for i in {1..30}
do
	echo "Queueing run #$i"
	qsub 7_tool.pbs
	qsub 7_tool_nosto.pbs
    	qsub 7_tool_stock.pbs
    	qsub 7_machine.pbs
	qsub 7_machine_nosto.pbs
    	qsub 7_machine_stock.pbs
    	qsub 7_factory.pbs
	qsub 7_factory_nosto.pbs
    	qsub 7_factory_stock.pbs
    	qsub 7_Industry.pbs
	qsub 7_Industry_nosto.pbs
    	qsub 7_Industry_stock.pbs
done



cd ../../WithoutDevDev/1
for i in {1..30}
do
	echo "Queueing run #$i"
	qsub 1_tool.pbs
	qsub 1_tool_nosto.pbs
    	qsub 1_tool_stock.pbs
    	qsub 1_machine.pbs
	qsub 1_machine_nosto.pbs
    	qsub 1_machine_stock.pbs
    	qsub 1_factory.pbs
	qsub 1_factory_nosto.pbs
    	qsub 1_factory_stock.pbs
    	qsub 1_Industry.pbs
	qsub 1_Industry_nosto.pbs
    	qsub 1_Industry_stock.pbs
done

cd ../3
for i in {1..30}
do
	echo "Queueing run #$i"
	qsub 3_tool.pbs
	qsub 3_tool_nosto.pbs
    	qsub 3_tool_stock.pbs
    	qsub 3_machine.pbs
	qsub 3_machine_nosto.pbs
    	qsub 3_machine_stock.pbs
    	qsub 3_factory.pbs
	qsub 3_factory_nosto.pbs
    	qsub 3_factory_stock.pbs
    	qsub 3_Industry.pbs
	qsub 3_Industry_nosto.pbs
    	qsub 3_Industry_stock.pbs
done

cd ../5
for i in {1..30}
do
	echo "Queueing run #$i"
    	qsub 5_none.pbs
	qsub 5_none_nosto.pbs
    	qsub 5_none_stock.pbs
	qsub 5_tool.pbs
	qsub 5_tool_nosto.pbs
    	qsub 5_tool_stock.pbs
    	qsub 5_machine.pbs
	qsub 5_machine_nosto.pbs
    	qsub 5_machine_stock.pbs
    	qsub 5_factory.pbs
	qsub 5_factory_nosto.pbs
    	qsub 5_factory_stock.pbs
    	qsub 5_Industry.pbs
	qsub 5_Industry_nosto.pbs
    	qsub 5_Industry_stock.pbs
done

cd ../7
for i in {1..30}
do
	echo "Queueing run #$i"
	qsub 7_tool.pbs
	qsub 7_tool_nosto.pbs
    	qsub 7_tool_stock.pbs
    	qsub 7_machine.pbs
	qsub 7_machine_nosto.pbs
    	qsub 7_machine_stock.pbs
    	qsub 7_factory.pbs
	qsub 7_factory_nosto.pbs
    	qsub 7_factory_stock.pbs
    	qsub 7_Industry.pbs
	qsub 7_Industry_nosto.pbs
    	qsub 7_Industry_stock.pbs
done
