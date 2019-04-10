import modules_amds_df as magpie


if __name__ == '__main__':
    filepath = "/Users/mac/Dropbox/magpie/datastore/"
    label = 0

    if label == 0:
        test = ['0.1', '0.2', '0.3', '0.4', '0.5', '0.6', '0.7']
    else:
        test = ['1.1', '1.2', '1.3', '1.4', '1.5', '1.6', '1.7']


    action_set = [
        [['ip'], ['wifi'], ['zigbee'],
         ['rf'], ['audio'], ['amds']]
    ]

    presence_extract = magpie.Model()

    for i in test:
        presence_extract.test(filepath, i, action_set[0], label)

