from enum import StrEnum
from chip import ChipDeviceCtrl
from chip.clusters.Types import NullValue
import chip.clusters as Clusters


class Bottle(StrEnum):
    kBourbon = "bourbon"
    kGin = "gin"
    kCampari = "campari"
    kVermouth = "vermouth"
    kSourMix = "sour mix"
    kSimpleSyrup = "simple syrup",


class Oz:
    def __init__(self, oz: float):
        self.oz = oz

    def time(self):
        # One oz == approx 12s.
        return self.oz * 12


class DrinkMachine:
    def __init__(self, devCtrl: ChipDeviceCtrl, node_id: int):
        self.dev_ctrl = devCtrl
        self.node_id = node_id
        # TODO: Should this actually be modelled as an aggregator with bridged nodes so I can have a NodeLabel?
        # Right now I'm going to leave this as something on the app side because it's a demo, but it's weird that we don't have writeable labels unless you model it strangely. Spec issue?
        self.bottles: dict[Bottle, int] = {Bottle.kBourbon: 1, Bottle.kGin: 2,
                                           Bottle.kCampari: 3, Bottle.kVermouth: 4, Bottle.kSourMix: 5, Bottle.kSimpleSyrup: 6}
        self.recipes: dict[str, dict[Bottle, Oz]] = {}
        self.add_recipe("negroni", {Bottle.kGin: Oz(1.5), Bottle.kCampari: Oz(1.5), Bottle.kVermouth: Oz(1.5)})
        self.add_recipe("boulevardier", {Bottle.kBourbon: Oz(1.5), Bottle.kCampari: Oz(1.5), Bottle.kVermouth: Oz(1.5)})
        self.add_recipe("bourbon sour", {Bottle.kBourbon: Oz(2), Bottle.kSourMix: Oz(1), Bottle.kSimpleSyrup: Oz(1.5)})
        self.add_recipe("martini", {Bottle.kGin: Oz(2), Bottle.kVermouth: Oz(0.25)})
        self.add_recipe("gimlet", {Bottle.kGin: Oz(2.5), Bottle.kSourMix: Oz(0.5), Bottle.kSimpleSyrup: Oz(0.5)})
        self.add_recipe("old fashioned", {Bottle.kBourbon: Oz(2), Bottle.kSimpleSyrup: Oz(0.125)})
        self.add_recipe("shot of bourbon", {Bottle.kBourbon: Oz(1.5)})
        self.add_recipe("shot of gin", {Bottle.kGin: Oz(1.5)})

    def set_bottle_names(self, bottles: dict[Bottle, int]) -> bool:
        ''' Bottle is a dict of bottle name to endpoint and should contain all 6 endpoints at once'''
        if len(bottles) != 6:
            return False
        self.bottles = bottles

    def get_bottle_names(self):
        return self.bottles

    def get_recipes(self):
        return self.recipes

    def add_recipe(self, name: str, ingredients: dict[Bottle, Oz]):
        # TODO: should store somewhere permanent - simplest is to write out to file. In the meanwhile, we have a few pre-populated
        self.recipes[name] = ingredients

    async def dispense(self, recipe: str):
        # TODO: be a bit nicer on the comparison here. Strings as keys aren't great, but I want the flexibility to add non-standard recipes
        if recipe not in self.recipes.keys():
            print(f"Unable to find the specified recipe. Available Recipes: {self.recipes.keys()}")
            return
        required_bottles = set(self.recipes[recipe].keys())
        if not required_bottles.issubset(set(self.bottles)):
            print('Recipe requires an ingredient that is not loaded into the drink machine')
            print(f'Recipe requires: {required_bottles}')
            print(f'Available: {self.bottles}')
            return

        ingredients = self.recipes[recipe]
        for bottle, amount in ingredients.items():
            ep = self.bottles[bottle]
            time = amount.time()
            await self.dev_ctrl.SendCommand(nodeid=self.node_id, endpoint=ep,
                                            payload=Clusters.ValveConfigurationAndControl.Commands.Open(openDuration=time))

    async def prime(self, endpoint: int):
        await self.dev_ctrl.SendCommand(nodeid=self.node_id, endpoint=endpoint,
                                        payload=Clusters.ValveConfigurationAndControl.Commands.Open(openDuration=NullValue))

    async def stop(self, endpoint: int):
        await self.dev_ctrl.SendCommand(nodeid=self.node_id, endpoint=endpoint,
                                        payload=Clusters.ValveConfigurationAndControl.Commands.Close())
