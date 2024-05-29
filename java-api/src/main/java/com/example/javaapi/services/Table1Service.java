package com.example.javaapi.services;

import com.example.javaapi.db.Table1;
import com.example.javaapi.repositories.Table1Repository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class Table1Service {

    @Autowired
    private Table1Repository table1Repository;

    public List<Table1> findAll() {
        return table1Repository.findAll();
    }

    public List<Table1> findByColumns(String columnA, String columnB, String columnC, String columnD) {
        return table1Repository.findByColumns(columnA, columnB, columnC, columnD);
    }

    public void deleteByColumns(String columnA, String columnB, String columnC, String columnD) {
        table1Repository.deleteByColumns(columnA, columnB, columnC, columnD);
    }
    public void updateByColumnAAndColumnB(String columnA, String columnB, String newColumnC, String newColumnD) {
        table1Repository.updateByColumnAAndColumnB(columnA, columnB, newColumnC, newColumnD);
    }
    public Table1 save(Table1 table1) {
        return table1Repository.save(table1);
    }
}
