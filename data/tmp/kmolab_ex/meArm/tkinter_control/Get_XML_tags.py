# from https://stackoverflow.com/questions/29596584/getting-a-list-of-xml-tags-in-file-using-xml-etree-elementtree
import xml.etree.ElementTree as ET
import re

def namespace(element):
    m = re.match(r'\{.*\}', element.tag)
    return m.group(0) if m else ''

def first(iterable, condition = lambda x: True):
    """
    from https://stackoverflow.com/questions/2361426/get-the-first-item-from-an-iterable-that-matches-a-condition
    Returns the first item in the `iterable` that
    satisfies the `condition`.

    If the condition is not given, returns the first item of
    the iterable.

    Raises `StopIteration` if no item satysfing the condition is found.

    >>> first( (1,2,3), condition=lambda x: x % 2 == 0)
    2
    >>> first(range(3, 100))
    3
    >>> first( () )
    Traceback (most recent call last):
    ...
    StopIteration
    """
    return next(x for x in iterable if condition(x))

def leoprint(data):
    try:
        print(data)
    except:
        g.es(data)
        
# load and parse the file
xmlTree = ET.parse('uarm_nx_tkinter_gui_control_exhaustive.simscene.xml')

# get the XML file namespace from the first element of the iterable
ns = namespace(first(xmlTree.iter()))

elemList = []

for elem in xmlTree.iter():
    elemList.append(elem.tag)


# now I remove duplicities - by convertion to set and back to list
elemList = list(set(elemList))

# Just printing out the result
#leoprint(len(elemList))

for i in elemList:
    # remove the namespace from the element tag
    leoprint(i.replace(ns, ""))