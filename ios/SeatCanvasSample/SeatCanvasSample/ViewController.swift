//
//  ViewController.swift
//  SeatCanvasSample
//
//  Created by king on 2025/11/12.
//

import UIKit

class ViewController: UIViewController {
    var tableView: UITableView!

    var baseMapSections: [[BaseMapFileInfo]] = []

    override func viewDidLoad() {
        super.viewDidLoad()

        title = "底图"

        baseMapSections.append(Bundle.Sample.defaultBaseMaps())
        baseMapSections.append(Bundle.Sample.customizedBaseMaps())

        setupTableView()
    }
}

extension ViewController: UITableViewDataSource, UITableViewDelegate {
    func setupTableView() {
        tableView = UITableView(frame: .zero, style: .plain)
        tableView.backgroundColor = .white
        tableView.translatesAutoresizingMaskIntoConstraints = false
        tableView.register(UITableViewCell.classForCoder(), forCellReuseIdentifier: "cell")
        view.addSubview(tableView)

        NSLayoutConstraint.activate([
            tableView.leadingAnchor.constraint(equalTo: view.leadingAnchor),
            tableView.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor),
            tableView.trailingAnchor.constraint(equalTo: view.trailingAnchor),
            tableView.bottomAnchor.constraint(equalTo: view.bottomAnchor),
        ])

        tableView.dataSource = self
        tableView.delegate = self
    }

    func numberOfSections(in _: UITableView) -> Int {
        baseMapSections.count
    }

    func tableView(_: UITableView, numberOfRowsInSection section: Int) -> Int {
        baseMapSections[section].count
    }

    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        guard let cell = tableView.dequeueReusableCell(withIdentifier: "cell") else {
            fatalError("Unregistered cell")
        }

        cell.textLabel?.text = baseMapSections[indexPath.section][indexPath.row].filename
        return cell
    }

    func tableView(_: UITableView, didSelectRowAt indexPath: IndexPath) {
        tableView.deselectRow(at: indexPath, animated: true)
        let baseMap = baseMapSections[indexPath.section][indexPath.row]
        let vc = SeatCanvasViewController(baseMap: baseMap)
        navigationController?.pushViewController(vc, animated: true)
    }
}
