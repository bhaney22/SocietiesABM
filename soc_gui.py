'''
The soc_gui module includes the definition of the App class.
The App class is instantiated for the user interface.
To enter the values for config files. Can load and save a file.
'''

from Tkinter import *
from tkFileDialog import asksaveasfilename, askopenfilename
from optparse import OptionParser
import shlex

class App:
    
    def __init__(self, master):
        
        """Set the name, location of each box"""
        frame = Frame(master)
        frame.grid(column=0, row=0)
        
        spacing1 = Label(frame, text="           ")
        spacing2 = Label(frame, text="           ")
        spacing1.grid(column=2, row=0)
        spacing2.grid(column=5, row=0)

        simTitle = Label(frame, text="Simulation variables", font='bold')
        simTitle.grid(column=0, row=0)

        self.startDayVar = IntVar(master)
        startDayLabel = Label(frame, text="Start Day:")
        startDayLabel.grid(column=0, row=1, sticky='e')
        startDayBox = Entry(frame, textvariable=self.startDayVar, width=7)
        startDayBox.grid(column=1, row=1, sticky=E)
        self.startDayError = Label(frame, fg='red', text=" ")
        self.startDayError.grid(column=2, row=1, sticky='w')

        self.dayLenthVar = IntVar(master)
        dayLenthLabel = Label(frame, text="Length of Days:")
        dayLenthLabel.grid(column=0, row=2, sticky='e')
        dayLenthBox = Entry(frame, textvariable=self.dayLenthVar, width=7)
        dayLenthBox.grid(column=1, row=2, sticky=E)
        self.dayLenthError = Label(frame, fg='red', text=" ")
        self.dayLenthError.grid(column=2, row=2, sticky='w')


        self.dayNumVar = IntVar(master)
        dayNumLabel = Label(frame, text="Number of Days:")
        dayNumLabel.grid(column=0, row=4, sticky='e')
        dayNumBox = Entry(frame, textvariable=self.dayNumVar, width=7)
        dayNumBox.grid(column=1, row=4)
        self.dayNumError = Label(frame, fg='red', text=" ")
        self.dayNumError.grid(column=2, row=4, sticky='w')

        self.agentVar = IntVar(master)
        agentLabel = Label(frame, text="Number of Agents:")
        agentLabel.grid(column=0, row=5, sticky='e')
        agentBox = Entry(frame, textvariable=self.agentVar, width=7)
        agentBox.grid(column=1, row=5)
        self.agentError = Label(frame, fg='red', text=" ")
        self.agentError.grid(column=2, row=5, sticky='w')


        self.resVar = IntVar(master)
        resLabel = Label(frame, text="Number of Resources:")
        resLabel.grid(column=0, row=6, sticky='e')
        resBox = Entry(frame, textvariable=self.resVar, width=7)
        resBox.grid(column=1, row=6)
        self.resError = Label(frame, fg='red', text=" ")
        self.resError.grid(column=2, row=6, sticky='w')

        self.typeVar = IntVar(master)
        typeLabel = Label(frame, text="Number of Types/Groups:")
        typeLabel.grid(column=0, row=7, sticky='e')
        typeBox = Entry(frame, textvariable=self.typeVar, width=7)
        typeBox.grid(column=1, row=7)
        self.resError = Label(frame, fg='red', text=" ")
        self.resError.grid(column=2, row=7, sticky='w')


        boolTitle = Label(frame, text="Input booleans", font='bold')
        boolTitle.grid(column=0, row=9)

        self.tradeVar = BooleanVar(master)
        self.tradeVar.trace("w", self.tradeExists)
        self.tradeCheck = Checkbutton(frame, text="trade exists", onvalue=True, offvalue=False, variable=self.tradeVar)
        self.tradeCheck.grid(column=0, row=10, sticky='e')

        self.deviceVar = BooleanVar(master)
        self.deviceVar.trace("w", self.deviceExists)
        self.deviceCheck = Checkbutton(frame, text="devices exists", onvalue=True, offvalue=False, variable=self.deviceVar)
        self.deviceCheck.grid(column=0, row=11, sticky='e')

        self.toolOnlyVar = BooleanVar(master)
        self.toolOnlyCheck = Checkbutton(frame, text="tools only", onvalue=True, offvalue=False, variable=self.toolOnlyVar)
        self.toolOnlyCheck.grid(column=0, row=12, sticky='e')


        tradeTitle = Label(frame, text="Trade Context variables", font='bold')
        tradeTitle.grid(column=0, row=14)

        self.menuSizeVar = IntVar(master)
        menuSizeLabel = Label(frame, text="Menu Size:")
        menuSizeLabel.grid(column=0, row=15, sticky='e')
        menuSizeBox = Entry(frame, textvariable=self.menuSizeVar, width=7)
        menuSizeBox.grid(column=1, row=15)
        self.menuSizeError = Label(frame, fg='red', text=" ")
        self.menuSizeError.grid(column=2, row=15, sticky='w')

        self.resTradeRoundVar = IntVar(master)
        resTradeRoundLabel = Label(frame, text="resource trade rounds:")
        resTradeRoundLabel.grid(column=0, row=16, sticky='e')
        resTradeRoundBox = Entry(frame, textvariable=self.resTradeRoundVar, width=7)
        resTradeRoundBox.grid(column=1, row=16)
        self.resTradeRoundError = Label(frame, fg='red', text=" ")
        self.resTradeRoundError.grid(column=2, row=16, sticky='w')

        self.resTradeAttemptVar = IntVar(master)
        resTradeAttemptLabel = Label(frame, text="resource trade attempts:")
        resTradeAttemptLabel.grid(column=0, row=17, sticky='e')
        resTradeAttemptBox = Entry(frame, textvariable=self.resTradeAttemptVar, width=7)
        resTradeAttemptBox.grid(column=1, row=17)
        self.resTradeAttemptError = Label(frame, fg='red', text=" ")
        self.resTradeAttemptError.grid(column=2, row=17, sticky='w')

        self.devTradeRoundVar = IntVar(master)
        devTradeRoundLabel = Label(frame, text="device trade rounds:")
        devTradeRoundLabel.grid(column=0, row=18, sticky='e')
        devTradeRoundBox = Entry(frame, textvariable=self.devTradeRoundVar, width=7)
        devTradeRoundBox.grid(column=1, row=18)
        self.devTradeRoundError = Label(frame, fg='red', text=" ")
        self.devTradeRoundError.grid(column=2, row=18, sticky='w')

        self.devTradeAttemptVar = IntVar(master)
        devTradeAttemptLabel = Label(frame, text="device trade attempts:")
        devTradeAttemptLabel.grid(column=0, row=19, sticky='e')
        devTradeAttemptBox = Entry(frame, textvariable=self.devTradeAttemptVar, width=7)
        devTradeAttemptBox.grid(column=1, row=19)
        self.devTradeAttemptError = Label(frame, fg='red', text=" ")
        self.devTradeAttemptError.grid(column=2, row=19, sticky='w')


        """
        self.removeAgentVar = BooleanVar(master)
        self.removeAgentVar.trace("w", self.removeAgent)
        self.removeAgentCheck = Checkbutton(frame, text="Remove an Agent", onvalue=True, offvalue=False, variable=self.removeAgentVar)
        self.removeAgentCheck.grid(column=3, row=21, sticky='e')
        self.removeAgentNumVar = IntVar(master)
        removeAgentNumLabel = Label(frame, text="Remove Agent Number:")
        removeAgentNumLabel.grid(column=3, row=22, sticky='e')
        self.removeAgentNumBox = Entry(frame, textvariable=self.removeAgentNumVar, width=7)
        self.removeAgentNumBox.grid(column=4, row=22)
        self.removeAgentNumError = Label(frame, fg='red', text=" ")
        self.removeAgentNumError.grid(column=5, row=22, sticky='w')
        self.removeAgentdayVar = IntVar(master)
        removeAgentdayLabel = Label(frame, text="Remove on Day Number:")
        removeAgentdayLabel.grid(column=3, row=23, sticky='e')
        self.removeAgentdayBox = Entry(frame, textvariable=self.removeAgentdayVar, width=7)
        self.removeAgentdayBox.grid(column=4, row=23)
        self.removeAgentdayError = Label(frame, fg='red', text=" ")
        self.removeAgentdayError.grid(column=5, row=23, sticky='w')

        self.removeResVar = BooleanVar(master)
        self.removeResVar.trace("w", self.removeRes)
        self.removeResCheck = Checkbutton(frame, text="Remove Resource", onvalue=True, offvalue=False, variable=self.removeResVar)
        self.removeResCheck.grid(column=0, row=18, sticky='e')
        self.removeResNumVar = IntVar(master)
        removeResNumLabel = Label(frame, text="Remove Resouce Number:")
        removeResNumLabel.grid(column=0, row=19, sticky='e')
        self.removeResNumBox = Entry(frame, textvariable=self.removeResNumVar, width=7)
        self.removeResNumBox.grid(column=1, row=19)
        self.removeResNumError = Label(frame, fg='red', text=" ")
        self.removeResNumError.grid(column=2, row=19, sticky='w')
        self.removeResDayVar = IntVar(master)
        removeResDayLabel = Label(frame, text="Remove on Day Number:")
        removeResDayLabel.grid(column=0, row=20, sticky='e')
        self.removeResDayBox = Entry(frame, textvariable=self.removeResDayVar, width=7)
        self.removeResDayBox.grid(column=1, row=20)
        self.removeResDayError = Label(frame, fg='red', text=" ")
        self.removeResDayError.grid(column=2, row=20, sticky='w')
        self.removeResResidue = BooleanVar(master)
        self.removeResResidueCheck = Checkbutton(frame, text="Remove Residue", onvalue=True, offvalue=False, variable=self.removeResResidue)
        self.removeResResidueCheck.grid(column=0, row=21, sticky='e')
        """



        agentCapTitle = Label(frame, text="Agent Capacity variables", font='bold')
        agentCapTitle.grid(column=3, row=0)


        self.devTradeMemLenVar = IntVar(master)
        devTradeMemLenLabel = Label(frame, text="Device trade memory length:")
        devTradeMemLenLabel.grid(column=3, row=1, sticky='e')
        devTradeMemLenBox = Entry(frame, textvariable=self.devTradeMemLenVar, width=7)
        devTradeMemLenBox.grid(column=4, row=1)
        self.devTradeMemLenError = Label(frame, fg='red', text=" ")
        self.devTradeMemLenError.grid(column=5, row=1, sticky='w')

        self.devProdMemLenVar = IntVar(master)
        devProdMemLenLabel = Label(frame, text="Device production memory length:")
        devProdMemLenLabel.grid(column=3, row=2, sticky='e')
        devProdMemLenBox = Entry(frame, textvariable=self.devProdMemLenVar, width=7)
        devProdMemLenBox.grid(column=4, row=2)
        self.devProdMemLenError = Label(frame, fg='red', text=" ")
        self.devProdMemLenError.grid(column=5, row=2, sticky='w')

        self.expLossVar = DoubleVar(master)
        expLossLabel = Label(frame, text="Daily experience loss for idle resources:")
        expLossLabel.grid(column=3, row=3, sticky='e')
        expLossBox = Entry(frame, textvariable=self.expLossVar, width=7)
        expLossBox.grid(column=4, row=3)
        self.expLossError = Label(frame, fg='red', text=" ")
        self.expLossError.grid(column=5, row=3, sticky='w')

        self.maxExpVar = DoubleVar(master)
        maxExpLabel = Label(frame, text="Maximum resource experence:")
        maxExpLabel.grid(column=3, row=4, sticky='e')
        maxExpBox = Entry(frame, textvariable=self.maxExpVar, width=7)
        maxExpBox.grid(column=4, row=4)
        self.maxExpError = Label(frame, fg='red', text=" ")
        self.maxExpError.grid(column=5, row=4, sticky='w')

        self.minEffVar = DoubleVar(master)
        minEffLabel = Label(frame, text="Minimum effort for a resource:")
        minEffLabel.grid(column=3, row=5, sticky='e')
        minEffBox = Entry(frame, textvariable=self.minEffVar, width=7)
        minEffBox.grid(column=4, row=5)
        self.minEffError = Label(frame, fg='red', text=" ")
        self.minEffError.grid(column=5, row=5, sticky='w')

        self.daysDevVar = DoubleVar(master)
        daysDevLabel = Label(frame, text="Days of device to hold:")
        daysDevLabel.grid(column=3, row=6, sticky='e')
        daysDevBox = Entry(frame, textvariable=self.daysDevVar, width=7)
        daysDevBox.grid(column=4, row=6)
        self.daysDevError = Label(frame, fg='red', text=" ")
        self.daysDevError.grid(column=5, row=6, sticky='w')


        ProdEfficTitle = Label(frame, text="Production Efficiency variables", font='bold')
        ProdEfficTitle.grid(column=3, row=8)

        self.resInToolVar = IntVar(master)
        resInToolLabel = Label(frame, text="Number of resources to make a tool:")
        resInToolLabel.grid(column=3, row=9, sticky='e')
        resInToolBox = Entry(frame, textvariable=self.resInToolVar, width=7)
        resInToolBox.grid(column=4, row=9)
        self.resInToolError = Label(frame, fg='red', text=" ")
        self.resInToolError.grid(column=5, row=9, sticky='w')


        self.inventExpVar = DoubleVar(master)
        inventExpLabel = Label(frame, text="Experence the inventor gains:")
        inventExpLabel.grid(column=3, row=10, sticky='e')
        inventExpBox = Entry(frame, textvariable=self.inventExpVar, width=7)
        inventExpBox.grid(column=4, row=10)
        self.inventExpError = Label(frame, fg='red', text=" ")
        self.inventExpError.grid(column=5, row=10, sticky='w')

        self.compInDev = IntVar(master)
        compInDevLabel = Label(frame, text="Number of components that go into a device:")
        compInDevLabel.grid(column=3, row=11, sticky='e')
        compInDevBox = Entry(frame, textvariable=self.compInDev, width=7)
        compInDevBox.grid(column=4, row=11)
        self.compInDevError = Label(frame, fg='red', text=" ")
        self.compInDevError.grid(column=5, row=11, sticky='w')

        self.maxDevExpVar = DoubleVar(master)
        maxDevExpLabel = Label(frame, text="Maximum experence in a device:")
        maxDevExpLabel.grid(column=3, row=12, sticky='e')
        maxDevExpBox = Entry(frame, textvariable=self.maxDevExpVar, width=7)
        maxDevExpBox.grid(column=4, row=12)
        self.maxDevExpError = Label(frame, fg='red', text=" ")
        self.maxDevExpError.grid(column=5, row=12, sticky='w')

        self.devDecay = DoubleVar(master)
        devDecayLabel = Label(frame, text="Device decay:")
        devDecayLabel.grid(column=3, row=13, sticky='e')
        devDecayBox = Entry(frame, textvariable=self.devDecay, width=7)
        devDecayBox.grid(column=4, row=13)
        self.devDecayError = Label(frame, fg='red', text=" ")
        self.devDecayError.grid(column=5, row=13, sticky='w')


        volSpeedTitle = Label(frame, text="Innovation Volume/Speed variables", font='bold')
        volSpeedTitle.grid(column=3, row=15)

        self.minResDevVar = IntVar(master)
        minResDevLabel = Label(frame, text="Minimum resources for device construction:")
        minResDevLabel.grid(column=3, row=16, sticky='e')
        minResDevBox = Entry(frame, textvariable=self.minResDevVar, width=7)
        minResDevBox.grid(column=4, row=16)
        self.minResDevError = Label(frame, fg='red', text=" ")
        self.minResDevError.grid(column=5, row=16, sticky='w')

        self.minDevExpVar = DoubleVar(master)
        minDevExpLabel = Label(frame, text="Minimum device experience for an agent with one:")
        minDevExpLabel.grid(column=3, row=17, sticky='e')
        prodEpsLabelBox = Entry(frame, textvariable=self.minDevExpVar, width=7)
        prodEpsLabelBox.grid(column=4, row=17)
        self.prodEpsLabelError = Label(frame, fg='red', text=" ")
        self.prodEpsLabelError.grid(column=5, row=17, sticky='w')

        self.minDevDevVar = IntVar(master)
        minDevDevLabel1 = Label(frame, text="Minimum devices made recently to consider")
        minDevDevLabel1.grid(column=3, row=18, sticky='e')
        minDevDevLabel2 = Label(frame, text="obtaining a device making device:")
        minDevDevLabel2.grid(column=3, row=19, sticky='e')
        minDevDevBox = Entry(frame, textvariable=self.minDevDevVar, width=7)
        minDevDevBox.grid(column=4, row=19)
        self.minDevDevError = Label(frame, fg='red', text=" ")
        self.minDevDevError.grid(column=5, row=19, sticky='w')

        otherVarTitle = Label(frame, text="Other variables", font="bold")
        otherVarTitle.grid(column=6, row=0)

        self.prodEpsilonVar = DoubleVar(master)
        prodEpsilonLabel = Label(frame, text="production epsilon:")
        prodEpsilonLabel.grid(column=6, row=1, sticky='e')
        prodEpsilonBox = Entry(frame, textvariable=self.prodEpsilonVar, width=7)
        prodEpsilonBox.grid(column=7, row=1)
        self.prodEpsilonError = Label(frame, fg='red', text=" ")
        self.prodEpsilonError.grid(column=8, row=1, sticky='w')

        self.tradeEpsilonVar = DoubleVar(master)
        tradeEpsilonLabel = Label(frame, text="trade epsilon:")
        tradeEpsilonLabel.grid(column=6, row=2, sticky='e')
        tradeEpsilonBox = Entry(frame, textvariable=self.tradeEpsilonVar, width=7)
        tradeEpsilonBox.grid(column=7, row=2)
        self.tradeEpsilonError = Label(frame, fg='red', text=" ")
        self.tradeEpsilonError.grid(column=8, row=2, sticky='w')

        self.maxResEffortVar = DoubleVar(master)
        maxResEffortLabel = Label(frame, text="max res effort:")
        maxResEffortLabel.grid(column=6, row=3, sticky='e')
        maxResEffortBox = Entry(frame, textvariable=self.maxResEffortVar, width=7)
        maxResEffortBox.grid(column=7, row=3)
        self.maxResEffortError = Label(frame, fg='red', text=" ")
        self.maxResEffortError.grid(column=8, row=3, sticky='w')

        self.minResEffortVar = DoubleVar(master)
        minResEffortLabel = Label(frame, text="min res effort:")
        minResEffortLabel.grid(column=6, row=4, sticky='e')
        minResEffortBox = Entry(frame, textvariable=self.minResEffortVar, width=7)
        minResEffortBox.grid(column=7, row=4)
        self.minResEffortError = Label(frame, fg='red', text=" ")
        self.minResEffortError.grid(column=8, row=4, sticky='w')

        self.maxDevEffortVar = DoubleVar(master)
        maxDevEffortLabel = Label(frame, text="max device effort:")
        maxDevEffortLabel.grid(column=6, row=5, sticky='e')
        maxDevEffortBox = Entry(frame, textvariable=self.maxDevEffortVar, width=7)
        maxDevEffortBox.grid(column=7, row=5)
        self.maxDevEffortError = Label(frame, fg='red', text=" ")
        self.maxDevEffortError.grid(column=8, row=5, sticky='w')

        self.minDevEffortVar = DoubleVar(master)
        minDevEffortLabel = Label(frame, text="min device effort:")
        minDevEffortLabel.grid(column=6, row=6, sticky='e')
        minDevEffortBox = Entry(frame, textvariable=self.minDevEffortVar, width=7)
        minDevEffortBox.grid(column=7, row=6)
        self.minDevEffortError = Label(frame, fg='red', text=" ")
        self.minDevEffortError.grid(column=8, row=6, sticky='w')

        self.minResUtilVar = DoubleVar(master)
        minResUtilLabel = Label(frame, text="min resource util:")
        minResUtilLabel.grid(column=6, row=7, sticky='e')
        minResUtilBox = Entry(frame, textvariable=self.minResUtilVar, width=7)
        minResUtilBox.grid(column=7, row=7)
        self.minResUtilError = Label(frame, fg='red', text=" ")
        self.minResUtilError.grid(column=8, row=7, sticky='w')

        self.devProbFactorVar = DoubleVar(master)
        devProbFactorLabel = Label(frame, text="device probability factor:")
        devProbFactorLabel.grid(column=6, row=8, sticky='e')
        devProbFactorBox = Entry(frame, textvariable=self.devProbFactorVar, width=7)
        devProbFactorBox.grid(column=7, row=8)
        self.devProbFactorError = Label(frame, fg='red', text=" ")
        self.devProbFactorError.grid(column=8, row=8, sticky='w')

        self.toolProbFactorVar = DoubleVar(master)
        toolProbFactorLabel = Label(frame, text="tool probability factor:")
        toolProbFactorLabel.grid(column=6, row=9, sticky='e')
        toolProbFactorBox = Entry(frame, textvariable=self.toolProbFactorVar, width=7)
        toolProbFactorBox.grid(column=7, row=9)
        self.toolProbFactorError = Label(frame, fg='red', text=" ")
        self.toolProbFactorError.grid(column=8, row=9, sticky='w')

        extractEfficTitle = Label(frame, text="Extraction Efficiency variables", font='bold')
        extractEfficTitle.grid(column=8, row=0)

        self.factToolVar = DoubleVar(master)
        factToolLabel = Label(frame, text="Factor of a tool:")
        factToolLabel.grid(column=8, row=1, sticky='e')
        factToolBox = Entry(frame, textvariable=self.factToolVar, width=7)
        factToolBox.grid(column=9, row=1)
        self.factToolError = Label(frame, fg='red', text=" ")
        self.factToolError.grid(column=10, row=1, sticky='w')


        self.lifeToolVar = DoubleVar(master)
        lifeToolLabel = Label(frame, text="Lifetime of a tool:")
        lifeToolLabel.grid(column=8, row=2, sticky='e')
        lifeToolBox = Entry(frame, textvariable=self.lifeToolVar, width=7)
        lifeToolBox.grid(column=9, row=2)
        self.lifeToolError = Label(frame, fg='red', text=" ")
        self.lifeToolError.grid(column=10, row=2, sticky='w')

        self.factMachVar = DoubleVar(master)
        factMachLabel = Label(frame, text="Factor of a machine:")
        factMachLabel.grid(column=8, row=3, sticky='e')
        factMachBox = Entry(frame, textvariable=self.factMachVar, width=7)
        factMachBox.grid(column=9, row=3)
        self.factMachError = Label(frame, fg='red', text=" ")
        self.factMachError.grid(column=10, row=3, sticky='w')

        self.lifeMachVar = DoubleVar(master)
        lifeMachLabel = Label(frame, text="Lifetime of a machine:")
        lifeMachLabel.grid(column=8, row=4, sticky='e')
        lifeMachBox = Entry(frame, textvariable=self.lifeMachVar, width=7)
        lifeMachBox.grid(column=9, row=4)
        self.lifeMachError = Label(frame, fg='red', text=" ")
        self.lifeMachError.grid(column=10, row=4, sticky='w')

        self.factFactVar = DoubleVar(master)
        factFactLabel = Label(frame, text="Factor of a factory:")
        factFactLabel.grid(column=8, row=5, sticky='e')
        factFactBox = Entry(frame, textvariable=self.factFactVar, width=7)
        factFactBox.grid(column=9, row=5)
        self.factFactError = Label(frame, fg='red', text=" ")
        self.factFactError.grid(column=10, row=5, sticky='w')

        self.lifeFactVar = DoubleVar(master)
        lifeFactLabel = Label(frame, text="Lifetime of a factory:")
        lifeFactLabel.grid(column=8, row=6, sticky='e')
        lifeFactBox = Entry(frame, textvariable=self.lifeFactVar, width=7)
        lifeFactBox.grid(column=9, row=6)
        self.lifeFactError = Label(frame, fg='red', text=" ")
        self.lifeFactError.grid(column=10, row=6, sticky='w')

        self.FactIndVar = DoubleVar(master)
        factIndLabel = Label(frame, text="Factor of a industry:")
        factIndLabel.grid(column=8, row=7, sticky='e')
        factIndBox = Entry(frame, textvariable=self.FactIndVar, width=7)
        factIndBox.grid(column=9, row=7)
        self.factIndError = Label(frame, fg='red', text=" ")
        self.factIndError.grid(column=10, row=7, sticky='w')

        self.lifeIndVar = DoubleVar(master)
        lifeIndLabel = Label(frame, text="Lifetime of a industry:")
        lifeIndLabel.grid(column=8, row=8, sticky='e')
        lifeIndBox = Entry(frame, textvariable=self.lifeIndVar, width=7)
        lifeIndBox.grid(column=9, row=8)
        self.lifeIndError = Label(frame, fg='red', text=" ")
        self.lifeIndError.grid(column=10, row=8, sticky='w')

        self.factDevMechVar = DoubleVar(master)
        factDevMechLabel = Label(frame, text="Factor of a device machine:")
        factDevMechLabel.grid(column=8, row=9, sticky='e')
        factDevMechBox = Entry(frame, textvariable=self.factDevMechVar, width=7)
        factDevMechBox.grid(column=9, row=9)
        self.factDevMechError = Label(frame, fg='red', text=" ")
        self.factDevMechError.grid(column=10, row=9, sticky='w')

        self.lifeDevMechVar = DoubleVar(master)
        lifeDevMechLabel = Label(frame, text="Lifetime of a device machine:")
        lifeDevMechLabel.grid(column=8, row=10, sticky='e')
        lifeDevMechBox = Entry(frame, textvariable=self.lifeDevMechVar, width=7)
        lifeDevMechBox.grid(column=9, row=10)
        self.lifeDevMechError = Label(frame, fg='red', text=" ")
        self.lifeDevMechError.grid(column=10, row=10, sticky='w')

        self.factDevFactVar = DoubleVar(master)
        factDevFactLabel = Label(frame, text="Factor of a device factory:")
        factDevFactLabel.grid(column=8, row=11, sticky='e')
        factDevFactBox = Entry(frame, textvariable=self.factDevFactVar, width=7)
        factDevFactBox.grid(column=9, row=11)
        self.factDevFactError = Label(frame, fg='red', text=" ")
        self.factDevFactError.grid(column=10, row=11, sticky='w')

        self.lifeDevFactVar = DoubleVar(master)
        lifeDevFactLabel = Label(frame, text="Lifetime of a device factory:")
        lifeDevFactLabel.grid(column=8, row=12, sticky='e')
        lifeDevFactBox = Entry(frame, textvariable=self.lifeDevFactVar, width=7)
        lifeDevFactBox.grid(column=9, row=12)
        self.lifeDevFactError = Label(frame, fg='red', text=" ")
        self.lifeDevFactError.grid(column=10, row=12, sticky='w')


        self.savingVar = BooleanVar(master)
        self.savingVar.trace("w", self.saving)
        self.savingCheck = Checkbutton(frame, text="Save Results", onvalue=True, offvalue=False, variable=self.savingVar)
        self.savingCheck.grid(column=8, row=18, sticky='e')
        self.savingNameVar = StringVar(master)
        savingNameLabel = Label(frame, text="Name of the folder:")
        savingNameLabel.grid(column=8, row=19, sticky='e')
        self.savingNameBox = Entry(frame, textvariable=self.savingNameVar, width=7)
        self.savingNameBox.grid(column=9, row=19)
        self.savingNameError = Label(frame, fg='red', text=" ")
        self.savingNameError.grid(column=10, row=19, sticky='w')

        self.resetVar()

        self.currentFileLabel = Label(frame, text="Current file: %s:" % options.filename)
        self.currentFileLabel.grid(column=8, row=21, sticky='e')
        loadBut = Button(frame, command=self.loadFile, text="  LOAD  ")
        loadBut.grid(column=9, row=21)

        saveExitBut = Button(frame, command=self.saveToFile, text="     Save     ")
        saveExitBut.grid(column=8, row=22, sticky='e')

    def resetVar(self):
        global VARIABLE_INPUTS;
        VARIABLE_INPUTS = [('START_DAY', 'int'),
		          ('DAY_LENGTH', 'int'),
		          ('NUM_DAYS', 'int'),
		          ('NUM_AGENTS', 'int'),
		          ('NUM_RESOURCES', 'int'),
                  	  ('NUM_GROUPS', 'int'),
		          ('RES_TRADE_ROUNDS', 'int'),
		          ('RES_TRADE_ATTEMPTS', 'int'),
		          ('DEVICE_TRADE_ROUNDS', 'int'),
		          ('DEVICE_TRADE_ATTEMPTS', 'int'),
		          ('MENU_SIZE', 'int'),
		          ('DEVICE_TRADE_MEMORY_LENGTH', 'int'),
		          ('DEVICE_PRODUCTION_MEMORY_LENGTH', 'int'),
		          ('MIN_DEVICE_FOR_DEV_DEVICE_CONSIDERATION', 'int'),
		          ('MIN_RES_HELD_FOR_DEVICE_CONSIDERATION', 'int'),
		          ('DAILY_EXP_PENALTY', 'float'),
		          ('PRODUCTION_EPSILON', 'float'),
		          ('RESOURCES_IN_TOOL', 'int'),
                  ('MAX_RES_EXPERIENCE', 'float'),
		          ('INVENTOR_DEVICE_EXPERIENCE', 'float'),
		          ('NUM_DEVICE_COMPONENTS', 'int'),
	              ('DAILY_DEVICE_DECAY', 'float'),
	              ('MIN_HELD_DEVICE_EXPERIENCE', 'float'),
		          ('MIN_RES_UTIL', 'float'),
		          ('TRADE_EPSILON', 'float'),
		          ('TOOL_PROBABILITY_FACTOR', 'float'),
		          ('DEVICE_PROBABILITY_FACTOR', 'float'),
		          ('TOOL_FACTOR', 'float'),
		          ('TOOL_LIFETIME', 'float'),
		          ('MACHINE_FACTOR', 'float'),
		          ('MACHINE_LIFETIME', 'float'),
		          ('FACTORY_FACTOR', 'float'),
		          ('FACTORY_LIFETIME', 'float'),
		          ('INDUSTRY_FACTOR', 'float'),
		          ('INDUSTRY_LIFETIME', 'float'),
		          ('DEV_MACHINE_FACTOR', 'float'),
		          ('DEV_MACHINE_LIFETIME', 'float'),
		          ('DEV_FACTORY_FACTOR', 'float'),
		          ('DEV_FACTORY_LIFETIME', 'float'),
		          ('DAYS_OF_DEVICE_TO_HOLD', 'float'),
                  ('OTHER_MARKETS', 'bool'),
                  ('MIN_RES_EFFORT', 'float'),
                  ('MAX_RES_EFFORT', 'float'),
                  ('MAX_DEVICE_EFFORT', 'float'),
                  ('MIN_DEVICE_EFFORT', 'float'),
                  ('MAX_DEVICE_EXPERIENCE', 'float'),
		          ('TRADE_EXISTS', 'bool'),
		          ('DEVICES_EXIST', 'bool'),
		          ('TOOLS_ONLY', 'bool')]

        config = open(options.filename, 'r')
        for line in config:
            for eachVariable in VARIABLE_INPUTS:
                if eachVariable[0] in line:
                    splittedLine = shlex.split(line)
                    exec("global " + eachVariable[0] + "; " + eachVariable[0] + " = " +eachVariable[1] + "(" + splittedLine[2] + ")")

        self.startDayVar.set(START_DAY)
        self.dayLenthVar.set(DAY_LENGTH)
        self.dayNumVar.set(NUM_DAYS)
        self.agentVar.set(NUM_AGENTS)
        self.resVar.set(NUM_RESOURCES)
        self.typeVar.set(NUM_GROUPS)

        self.menuSizeVar.set(MENU_SIZE)
        self.resTradeRoundVar.set(RES_TRADE_ROUNDS)
        self.resTradeAttemptVar.set(RES_TRADE_ATTEMPTS)
        self.devTradeRoundVar.set(DEVICE_TRADE_ROUNDS)
        self.devTradeAttemptVar.set(DEVICE_TRADE_ATTEMPTS)

        self.devTradeMemLenVar.set(DEVICE_TRADE_MEMORY_LENGTH)
        self.devProdMemLenVar.set(DEVICE_PRODUCTION_MEMORY_LENGTH)
        self.expLossVar.set(DAILY_EXP_PENALTY)
        self.maxExpVar.set(MAX_RES_EXPERIENCE)
        self.minEffVar.set(MIN_RES_EFFORT)
        self.daysDevVar.set(DAYS_OF_DEVICE_TO_HOLD)

        self.resInToolVar.set(RESOURCES_IN_TOOL)
        self.inventExpVar.set(INVENTOR_DEVICE_EXPERIENCE)
        self.compInDev.set(NUM_DEVICE_COMPONENTS)
        self.maxDevExpVar.set(MAX_DEVICE_EXPERIENCE)
        self.devDecay.set(DAILY_DEVICE_DECAY)

        self.minResDevVar.set(MIN_RES_HELD_FOR_DEVICE_CONSIDERATION)
        self.minDevExpVar.set(MIN_HELD_DEVICE_EXPERIENCE)
        self.minDevDevVar.set(MIN_DEVICE_FOR_DEV_DEVICE_CONSIDERATION)

        self.factToolVar.set(TOOL_FACTOR)
        self.lifeToolVar.set(TOOL_LIFETIME)
        self.factMachVar.set(MACHINE_FACTOR)
        self.lifeMachVar.set(MACHINE_LIFETIME)
        self.factFactVar.set(FACTORY_FACTOR)
        self.lifeFactVar.set(FACTORY_LIFETIME)
        self.FactIndVar.set(INDUSTRY_FACTOR)
        self.lifeIndVar.set(INDUSTRY_LIFETIME)
        self.factDevFactVar.set(DEV_FACTORY_FACTOR)
        self.lifeDevFactVar.set(DEV_FACTORY_LIFETIME)
        self.factDevMechVar.set(DEV_MACHINE_FACTOR)
        self.lifeDevMechVar.set(DEV_MACHINE_LIFETIME)

        self.prodEpsilonVar.set(PRODUCTION_EPSILON)
        self.tradeEpsilonVar.set(TRADE_EPSILON)
        self.maxResEffortVar.set(MAX_RES_EFFORT)
        self.minResEffortVar.set(MIN_RES_EFFORT)
        self.maxDevEffortVar.set(MAX_DEVICE_EFFORT)
        self.minDevEffortVar.set(MIN_DEVICE_EFFORT)
        self.minResUtilVar.set(MIN_RES_UTIL)
        self.devProbFactorVar.set(DEVICE_PROBABILITY_FACTOR)
        self.toolProbFactorVar.set(TOOL_PROBABILITY_FACTOR)

        if TRADE_EXISTS:
            self.tradeCheck.select()
        else:
            self.tradeCheck.deselect()

        if DEVICES_EXIST:
            self.deviceCheck.select()
        else:
            self.deviceCheck.deselect()

        if TOOLS_ONLY:
            self.toolOnlyCheck.select()
        else:
            self.toolOnlyCheck.deselect()
            
        """
        if options.removeAgent:
            self.removeAgentCheck.select()
            self.removeAgentNumVar.set(options.removeAgent[0])
            self.removeAgentdayVar.set(options.removeAgent[1])
        else:
            self.removeAgentCheck.deselect()
            self.removeAgentNumVar.set(0)
            self.removeAgentdayVar.set(0)

        if options.removeRes:
            self.removeResCheck.select()
            self.removeResNumVar.set(options.removeRes[0])
            self.removeResDayVar.set(options.removeRes[1])
            if options.removeRes[2]:
                self.removeResResidueCheck.select()
            else:
                self.removeResResidueCheck.deselect()
        else:
            self.removeResCheck.deselect()
            self.removeResNumVar.set(0)
            self.removeResDayVar.set(0)
            self.removeResResidueCheck.deselect()
        
        if options.save:
            self.savingCheck.select()
            self.savingNameVar.set(options.save)
        else:
            self.savingCheck.deselect()
            self.savingNameVar.set('default')
        """

    """
    def removeAgent(self, *args):
        if self.removeAgentVar.get() == 0:
            self.removeAgentNumBox.configure(state='disabled')
            self.removeAgentNumError.config(text=" ")
            self.removeAgentdayBox.configure(state='disabled')
            self.removeAgentdayError.config(text=" ")
        else:
            self.removeAgentNumBox.configure(state='normal')

    def removeRes(self, *args):
        if self.removeResVar.get() == 0:
            self.removeResNumBox.configure(state='disabled')
            self.removeResNumError.config(text=" ")
            self.removeResDayBox.configure(state='disabled')
            self.removeResDayError.config(text=" ")
            self.removeResResidueCheck.configure(state='disabled')
        else:
            self.removeResNumBox.configure(state='normal')
            self.removeResDayBox.configure(state='normal')
            self.removeResResidueCheck.configure(state='normal')
    """

    def saving(self, *args):
        if self.savingVar.get() == 0:
            self.savingNameBox.configure(state='disabled')
            self.savingNameError.config(text=" ")
        else:
            self.savingNameBox.configure(state='normal')

    def tradeExists(self, *args):
        if self.tradeVar.get() == 0:
            self.deviceCheck.deselect()
            self.deviceCheck.configure(state='disabled')
        else:
            self.deviceCheck.configure(state='normal')

    def deviceExists(self, *args):
        if self.deviceVar.get() == 0:
            self.toolOnlyCheck.deselect()
            self.toolOnlyCheck.configure(state='disabled')
        else:
            self.toolOnlyCheck.configure(state='normal')

    def loadFile(self):
        temp = askopenfilename(filetypes=[("config files","*.conf"),("allfiles","*")], initialdir="../../conf")
        if temp == "":
            return False
        options.filename = temp
        self.resetVar()
        self.currentFileLabel.config(text="%s" % options.filename)

    def saveToFile(self):
        temp = asksaveasfilename(filetypes=[("config files","*.conf")], initialdir="../../conf")
        if temp == "":
            return False
        tempFile = open(temp, "w")
        tempFile.write("START_DAY = %d\n" % self.startDayVar.get())
        tempFile.write("DAY_LENGTH = %d\n" % self.dayLenthVar.get())
        tempFile.write("NUM_DAYS = %d\n" % self.dayNumVar.get())
        tempFile.write("NUM_AGENTS = %d\n" % self.agentVar.get())
        tempFile.write("NUM_RESOURCES = %d\n" % self.resVar.get())
        tempFile.write("NUM_GROUPS = %d\n" % self.typeVar.get())
        tempFile.write("RES_TRADE_ROUNDS = %d\n" % self.resTradeRoundVar.get())
        tempFile.write("RES_TRADE_ATTEMPTS = %d\n" % self.resTradeAttemptVar.get())
        tempFile.write("DEVICE_TRADE_ROUNDS = %d\n" % self.devTradeRoundVar.get())
        tempFile.write("DEVICE_TRADE_ATTEMPTS = %d\n" % self.devTradeAttemptVar.get())
        tempFile.write("MENU_SIZE = %d\n" % self.menuSizeVar.get())
        tempFile.write("DEVICE_TRADE_MEMORY_LENGTH = %d\n" % self.devTradeMemLenVar.get())
        tempFile.write("DEVICE_PRODUCTION_MEMORY_LENGTH = %d\n" % self.devProdMemLenVar.get())
        tempFile.write("MIN_DEVICE_FOR_DEV_DEVICE_CONSIDERATION = %d\n" % self.minDevDevVar.get())
        tempFile.write("MIN_RES_HELD_FOR_DEVICE_CONSIDERATION = %d\n" % self.minResDevVar.get())
        tempFile.write("DAILY_EXP_PENALTY = %f\n" % self.expLossVar.get())
        tempFile.write("PRODUCTION_EPSILON = %f\n" % self.prodEpsilonVar.get())
        tempFile.write("RESOURCES_IN_TOOL = %d\n" % self.resInToolVar.get())
        tempFile.write("MAX_RES_EXPERIENCE = %f\n" % self.maxExpVar.get())
        tempFile.write("INVENTOR_DEVICE_EXPERIENCE = %d\n" % self.inventExpVar.get())
        tempFile.write("NUM_DEVICE_COMPONENTS = %d\n" % self.compInDev.get())
        tempFile.write("MAX_DEVICE_EXPERIENCE = %f\n" % self.maxDevExpVar.get())
        tempFile.write("DAILY_DEVICE_DECAY = %d\n" % self.devDecay.get())
        tempFile.write("MIN_HELD_DEVICE_EXPERIENCE = %d\n" % self.minDevExpVar.get())
        tempFile.write("TRADE_EPSILON = %f\n" % self.tradeEpsilonVar.get())
        tempFile.write("TOOL_FACTOR = %f\n" % self.factToolVar.get())
        tempFile.write("TOOL_LIFETIME = %f\n" % self.lifeToolVar.get())
        tempFile.write("MACHINE_FACTOR = %f\n" % self.factMachVar.get())
        tempFile.write("MACHINE_LIFETIME = %f\n" % self.lifeMachVar.get())
        tempFile.write("FACTORY_FACTOR = %f\n" % self.factFactVar.get())
        tempFile.write("FACTORY_LIFETIME = %f\n" % self.lifeFactVar.get())
        tempFile.write("INDUSTRY_FACTOR = %f\n" % self.FactIndVar.get())
        tempFile.write("INDUSTRY_LIFETIME = %f\n" % self.lifeIndVar.get())
        tempFile.write("DEV_MACHINE_FACTOR = %f\n" % self.factDevMechVar.get())
        tempFile.write("DEV_MACHINE_LIFETIME = %f\n" % self.lifeDevMechVar.get())
        tempFile.write("DEV_FACTORY_FACTOR = %f\n" % self.factDevFactVar.get())
        tempFile.write("DEV_FACTORY_LIFETIME = %f\n" % self.lifeDevFactVar.get())
        tempFile.write("DAYS_OF_DEVICE_TO_HOLD = %d\n" % self.daysDevVar.get())
        tempFile.write("OTHER_MARKETS = False\n")
        if self.tradeVar.get() == 0:
            tempFile.write("TRADE_EXISTS = False\n")
        else:
            tempFile.write("TRADE_EXISTS = True\n")

        if self.deviceVar.get() == 0:
            tempFile.write("DEVICES_EXIST = False\n")
        else:
            tempFile.write("DEVICES_EXIST = True\n")

        tempFile.write("MAX_RES_EFFORT = %f\n" % self.maxResEffortVar.get())
        tempFile.write("MIN_RES_EFFORT = %f\n" % self.minResEffortVar.get())
        tempFile.write("MAX_DEVICE_EFFORT = %f\n" % self.maxDevEffortVar.get())
        tempFile.write("MIN_DEVICE_EFFORT = %f\n" % self.minDevEffortVar.get()) 
        tempFile.write("MIN_RES_UTIL = %f\n" % self.minResUtilVar.get())
        tempFile.write("TOOL_PROBABILITY_FACTOR = %f\n" % self.toolProbFactorVar.get())
        tempFile.write("DEVICE_PROBABILITY_FACTOR = %f\n" % self.devProbFactorVar.get())
        

        if self.toolOnlyVar.get() == 0:
            tempFile.write("TOOLS_ONLY = False\n")
        else:
            tempFile.write("TOOLS_ONLY = True\n")

        tempFile.close()
        options.filename = temp
        self.currentFileLabel.config(text="%s" % options.filename)
        return True


            
if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option("-p", "--parameter", dest="filename", default="../../conf/defaultValues.conf",
                          help="use given file for global variable values", metavar="FILE")
    (options, args) = parser.parse_args()
    GUI = Tk()
    GUI.title('SOCIETIES')
    app = App(GUI)
    GUI.mainloop()











